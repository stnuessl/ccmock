# 
# Copyright (C) 2023  Steffen Nuessle
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# 

add_custom_target(unit-tests)

set(
    UNIT_TEST_SOURCES
    src/MockAction.cpp
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
            --rc branch_coverage=1
            --rc function_coverage=1
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

