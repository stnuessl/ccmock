#
# The MIT License (MIT)
#
# Copyright (c) 2023  Steffen Nuessle
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

project(cxx-code CXX)

set(SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(
    CCMOCK_BACKENDS
    gmock
)

set(
    FILES
    rect.cpp
    value.cpp
    clone1.cpp
    clone2.cpp
    diffdecls.cpp
    #global.cpp
)

foreach(CCMOCK_BACKEND ${CCMOCK_BACKENDS})

    set(UNIT_TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CCMOCK_BACKEND})

    foreach(FILE ${FILES})
        set(SRC_FILE ${SOURCE_DIRECTORY}/${FILE})

        set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CCMOCK_BACKEND})
        set(INC_FILE ${OUTPUT_DIRECTORY}/${FILE})
        cmake_path(REPLACE_EXTENSION INC_FILE .inc)
        
        add_custom_command(
            OUTPUT ${INC_FILE}
            COMMAND ${CCMOCK_BIN}
                    -o ${INC_FILE}
                    --backend ${CCMOCK_BACKEND}
                    ${SRC_FILE}
            DEPENDS ccmock
                    ${SRC_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            PRE_BUILD
            VERBATIM
        )

        set(UT_FILE ${UNIT_TEST_DIRECTORY}/${FILE})

        cmake_path(GET FILE STEM FILE_STEM)
        set(TARGET cxx-${CCMOCK_BACKEND}-${FILE_STEM})

        add_executable(
            ${TARGET}
            EXCLUDE_FROM_ALL
            ${UT_FILE}
            ${INC_FILE}
        )

        set_target_properties(
            ${TARGET}
            PROPERTIES EXPORT_COMPILE_COMMANDS OFF
                       CXX_STANDARD 17
                       RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
        )
        
        set_property(
            TARGET ${TARGET}
            PROPERTY COMPILE_DEFINITIONS
        )

        set_property(
            TARGET ${TARGET}
            PROPERTY COMPILE_OPTIONS 
        )

        set_property(
            TARGET ${TARGET}
            PROPERTY LINK_OPTIONS 
        )

        target_compile_options(
            ${TARGET}
            PRIVATE
            -Wall
            -O2
            $<$<STREQUAL:${CCMOCK_BACKEND},gmock>:${GTEST_CFLAGS}>
        )

        target_include_directories(
            ${TARGET}
            PRIVATE
            ${OUTPUT_DIRECTORY}
        )

        target_link_libraries(
            ${TARGET}
            $<$<STREQUAL:${CCMOCK_BACKEND},gmock>:${LIB_GMOCK}>
            $<$<STREQUAL:${CCMOCK_BACKEND},gmock>:${LIB_GTEST}>
        )

        set(TARGET_OUTPUT ${OUTPUT_DIRECTORY}/${TARGET}.xml)

        set(
            COMMAND
            ${OUTPUT_DIRECTORY}/${TARGET}
            --gtest_output=xml:${TARGET_OUTPUT} 
        )

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
        append(TARBALL_FILES ${TARGET_OUTPUT} ${INC_FILE})
    endforeach()

endforeach()
