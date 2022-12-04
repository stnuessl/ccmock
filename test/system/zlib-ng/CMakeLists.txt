#
# The MIT License (MIT)
#
# Copyright (c) 2022  Steffen Nuessle
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

project(zlib-ng C CXX)

set(ZLIBNG_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src)
set(ZLIBNG_BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build)
set(ZLIBNG_COMPILATION_DATABASE ${ZLIBNG_BUILD_DIRECTORY}/compile_commands.json)

if (NOT EXISTS ${ZLIBNG_SOURCE_DIRECTORY})
    execute_process(
        COMMAND git clone
                --quiet
                --depth 1
                --branch 2.0.6
                --config advice.detachedHead=false
                https://github.com/zlib-ng/zlib-ng
                ${ZLIBNG_SOURCE_DIRECTORY}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} 
            -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON
            -S ${ZLIBNG_SOURCE_DIRECTORY}
            -B ${ZLIBNG_BUILD_DIRECTORY}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_QUIET
    COMMAND_ERROR_IS_FATAL ANY
)

set(
    CCMOCK_BACKENDS
    cmocka
    fff
    gmock
)

set(
    SOURCE_FILES
    ${ZLIBNG_SOURCE_DIRECTORY}/adler32.c
    ${ZLIBNG_SOURCE_DIRECTORY}/crc32.c
    ${ZLIBNG_SOURCE_DIRECTORY}/uncompr.c
)

foreach(CCMOCK_BACKEND ${CCMOCK_BACKENDS})

    set(UNIT_TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CCMOCK_BACKEND})

    foreach(SRC_FILE ${SOURCE_FILES})
        cmake_path(
            RELATIVE_PATH SRC_FILE
            BASE_DIRECTORY ${ZLIBNG_SOURCE_DIRECTORY}
            OUTPUT_VARIABLE SRC_FILE_RELATIVE_PATH
        )
        
        set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CCMOCK_BACKEND})
        set(INC_FILE ${OUTPUT_DIRECTORY}/${SRC_FILE_RELATIVE_PATH})
        cmake_path(REPLACE_EXTENSION INC_FILE .inc)

        add_custom_command(
            OUTPUT ${INC_FILE}
            COMMAND ${CCMOCK_BIN}
                    --compile-commands ${ZLIBNG_COMPILATION_DATABASE}
                    --backend ${CCMOCK_BACKEND}
                    -o ${INC_FILE}
                    ${SRC_FILE}
            DEPENDS ${CCMOCK_BIN}
                    ${ZLIBNG_COMPILATION_DATABASE}
                    ${SRC_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )

        cmake_path(GET SRC_FILE STEM SRC_FILE_STEM)
        set(UT_FILE ${UNIT_TEST_DIRECTORY}/${SRC_FILE_STEM})

        if(EXISTS ${UT_FILE}.cpp)
            set(UT_FILE ${UT_FILE}.cpp)
        elseif(EXISTS ${UT_FILE}.c)
            set(UT_FILE ${UT_FILE}.c)
        else()
            message(
                WARNING 
                "unable to find unit test implementation for \"${SRC_FILE}\"."
            )
        endif()

        set(TARGET zlibng-${CCMOCK_BACKEND}-${SRC_FILE_STEM})

        add_executable(
            ${TARGET}
            EXCLUDE_FROM_ALL
            ${SRC_FILE}
            ${UT_FILE}
            ${INC_FILE}
        )

        set_target_properties(
            ${TARGET}
            PROPERTIES EXPORT_COMPILE_COMMANDS OFF
                       CXX_STANDARD 17
                       RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
                       COMPILE_DEFINITIONS ""
                       COMPILE_OPTIONS ""
                       LINK_OPTIONS ""
        )
    
        # Retrieved from the generated compilation database
        target_compile_definitions(
            ${TARGET}
            PRIVATE
            NDEBUG
            HAVE_BUILTIN_CTZ
            HAVE_BUILTIN_CTZLL
            HAVE_POSIX_MEMALIGN
            HAVE_SYS_SDT_H
            HAVE_VISIBILITY_HIDDEN
            HAVE_VISIBILITY_INTERNAL
            UNALIGNED64_OK
            UNALIGNED_OK
            WITH_GZFILEOP
            X86_AVX2
            X86_AVX2_ADLER32
            X86_AVX_CHUNKSET
            X86_FEATURES
            X86_PCLMULQDQ_CRC
            X86_SSE2
            X86_SSE2_CHUNKSET
            X86_SSE2_SLIDEHASH
            X86_SSE42_CMP_STR
            X86_SSE42_CRC_HASH
            X86_SSE42_CRC_INTRIN
            X86_SSSE3
            X86_SSSE3_ADLER32
            ZLIB_DLL
            _LARGEFILE64_SOURCE=1
            __USE_LARGEFILE64
        )

        target_compile_options(
            ${TARGET}
            PRIVATE
            -Wall
            -O2
            $<$<STREQUAL:${CCMOCK_BACKEND},fff>:${GTEST_CFLAGS}>
            $<$<STREQUAL:${CCMOCK_BACKEND},gmock>:${GTEST_CFLAGS}>
        )

        target_include_directories(
            ${TARGET}
            PRIVATE
            ${ZLIBNG_SOURCE_DIRECTORY}
            ${ZLIBNG_BUILD_DIRECTORY}
            ${OUTPUT_DIRECTORY}
            $<$<STREQUAL:${CCMOCK_BACKEND},fff>:${FFF_INCLUDE_DIRECTORY}>
        )

        target_link_libraries(
            ${TARGET}
            $<$<STREQUAL:${CCMOCK_BACKEND},cmocka>:${LIB_CMOCKA}>
            $<$<STREQUAL:${CCMOCK_BACKEND},fff>:${LIB_GTEST}>
            $<$<STREQUAL:${CCMOCK_BACKEND},gmock>:${LIB_GMOCK}>
            $<$<STREQUAL:${CCMOCK_BACKEND},gmock>:${LIB_GTEST}>
        )

        set(TARGET_OUTPUT ${OUTPUT_DIRECTORY}/${TARGET}.xml)

        if (${CCMOCK_BACKEND} STREQUAL cmocka)
            set(
                COMMAND
                ${CMAKE_COMMAND} -E env
                CMOCKA_MESSAGE_OUTPUT=xml
                CMOCKA_XML_FILE=${TARGET_OUTPUT}
                ${OUTPUT_DIRECTORY}/${TARGET}
            )
        else()
            set(
                COMMAND
                ${OUTPUT_DIRECTORY}/${TARGET}
                --gtest_output=xml:${TARGET_OUTPUT} 
            )
        endif()

        if ($ENV{CI})
            set(COMMAND ${COMMAND} > /dev/null 2>&1)
        endif()

        add_custom_command(
            TARGET ${TARGET}
            COMMAND ${COMMAND}
            BYPRODUCTS ${TARGET_OUTPUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            POST_BUILD
            VERBATIM
        )

        add_dependencies(system-tests ${TARGET})
        add_dependencies(tarball ${TARGET})
        append(TARBALL_FILES ${TARGET_OUTPUT} ${INC_FILE})

    endforeach()
endforeach()

