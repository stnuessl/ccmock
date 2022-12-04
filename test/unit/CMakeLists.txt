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

add_custom_target(unit-tests)

set(
    UNIT_TEST_SOURCES
    src/ActionFactory.cpp
)

foreach(UT_SOURCE ${UNIT_TEST_SOURCES})

    cmake_path(GET UT_SOURCE STEM TARGET)
    cmake_path(GET UT_SOURCE PARENT_PATH UT_SOURCE_DIRECTORY)
    
    set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${UT_SOURCE_DIRECTORY})

    add_executable(
        ${TARGET}
        EXCLUDE_FROM_ALL
        ../../${UT_SOURCE}
        ${UT_SOURCE}
    )

    set_target_properties(
        ${TARGET}
        PROPERTIES EXPORT_COMPILE_COMMANDS OFF
                   CXX_STANDARD 17
                   RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    )
 
    target_compile_definitions(
        ${TARGET}
        PRIVATE UNIT_TESTS_ENABLED 
                $<TARGET_PROPERTY:${CMAKE_PROJECT_NAME},COMPILE_DEFINITIONS>
    )

    target_include_directories(
        ${TARGET}
        PRIVATE $<TARGET_PROPERTY:${CMAKE_PROJECT_NAME},INCLUDE_DIRECTORIES>
    )

    target_compile_options(
        ${TARGET} 
        PRIVATE ${GTEST_CFLAGS}
                -ftest-coverage
                -fprofile-arcs
                $<TARGET_PROPERTY:${CMAKE_PROJECT_NAME},COMPILE_OPTIONS>
    )

    target_link_options(
        ${TARGET}
        PRIVATE $<$<STREQUAL:$<CXX_COMPILER_ID>,Clang>:-coverage>
                $<TARGET_PROPERTY:${CMAKE_PROJECT_NAME},LINK_OPTIONS>
    )

    target_link_libraries(
        ${TARGET}
        PRIVATE $<$<STREQUAL:$<CXX_COMPILER_ID>,GNU>:gcov>
                ${LIB_GTEST} 
                ${LIB_GMOCK}
                $<TARGET_PROPERTY:${CMAKE_PROJECT_NAME},LINK_LIBRARIES>
    )

    set(TARGET_BIN ${OUTPUT_DIRECTORY}/${TARGET})
    set(TARGET_OUTPUT ${OUTPUT_DIRECTORY}/${TARGET}.xml)
    set(COMMAND ${TARGET} --gtest_output=xml:${TARGET_OUTPUT})

    if($ENV{CI})
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

    append(TARBALL_FILES ${TARGET_OUTPUT})
    add_dependencies(unit-tests ${TARGET})

endforeach()

set(
    UT_COVERAGE_INFO 
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-coverage.info
)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    # If called via the name gcov, llvm-cov will emulate gcov and can 
    # thus be directly used by lcov.
    find_program(LLVM_COV llvm-cov REQUIRED)
    file(CREATE_LINK ${LLVM_COV} ${CMAKE_CURRENT_BINARY_DIR}/gcov SYMBOLIC)

    set(LCOV_ARGS_CLANG --gcov-tool ${CMAKE_CURRENT_BINARY_DIR}/gcov)
endif()

add_custom_command(
    OUTPUT ${UT_COVERAGE_INFO}
    COMMAND lcov
            --base-directory ${CMAKE_SOURCE_DIR}
            --capture
            --directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles
            --include ${CMAKE_SOURCE_DIR}/*
            --exclude ${CMAKE_CURRENT_SOURCE_DIR}/*
            --output-file ${UT_COVERAGE_INFO}
            --rc lcov_branch_coverage=1
            --rc lcov_function_coverage=1
            ${LCOV_ARGS_CLANG}
    DEPENDS unit-tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

set(LCOV_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lcov)
set(LCOV_OUTPUT_HTML ${LCOV_OUTPUT_DIRECTORY}/index.html)

add_custom_command(
    OUTPUT ${LCOV_OUTPUT_HTML}
    COMMAND genhtml
            --branch-coverage
            --function-coverage
            --output-directory ${LCOV_OUTPUT_DIRECTORY}
            --rc geninfo_auto_base=1
            --show-details
            --legend
            --title ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}
            ${UT_COVERAGE_INFO}
    DEPENDS ${UT_COVERAGE_INFO}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_custom_target(lcov DEPENDS ${LCOV_OUTPUT_HTML})
add_dependencies(tarball lcov)

append(TARBALL_FILES ${LCOV_OUTPUT_DIRECTORY})

