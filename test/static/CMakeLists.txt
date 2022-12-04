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

include(cmake/clang-analysis.cmake)
include(cmake/cppcheck.cmake)
include(cmake/shellcheck.cmake)

find_program(CLANG clang ${CCMOCK_REQUIRE_DEPS})
find_program(CLANG_EXTDEF_MAPPING clang-extdef-mapping ${CCMOCK_REQUIRE_DEPS})
find_program(CPPCHECK cppcheck ${CCMOCK_REQUIRE_DEPS})
find_program(SHELLCHECK shellcheck ${CCMOCK_REQUIRE_DEPS})

set(CACHE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cache)

#
# clang-analysis
#
if (CLANG AND CLANG_EXTDEF_MAPPING)

    set(CLANG_ANALYSIS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/clang-analysis)

    add_clang_analysis(
        TARGET ${CMAKE_PROJECT_NAME}
        COMPILATION_DATABASE ${COMPILATION_DATABASE}
        CACHE_DIRECTORY ${CACHE_DIRECTORY}/clang-analysis
        OUTPUT_DIRECTORY ${CLANG_ANALYSIS_DIRECTORY}
        OUTPUT_FILES CLANG_ANALYSIS_OUTPUT
    )

    add_custom_target(clang-analysis DEPENDS ${CLANG_ANALYSIS_OUTPUT})
    add_dependencies(tarball clang-analysis)
    append(TARBALL_FILES ${CLANG_ANALYSIS_DIRECTORY})

endif()

#
# cppcheck
#
if (CPPCHECK)

    set(CPPCHECK_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cppcheck)

    add_cppcheck(
        TARGET ${CMAKE_PROJECT_NAME}
        COMPILATION_DATABASE ${COMPILATION_DATABASE}
        CACHE_DIRECTORY ${CACHE_DIRECTORY}/cppcheck
        OUTPUT_DIRECTORY ${CPPCHECK_DIRECTORY}
        OUTPUT_FILE CPPCHECK_OUTPUT
    )

    add_custom_target(cppcheck DEPENDS ${CPPCHECK_OUTPUT})
    add_dependencies(tarball cppcheck)
    append(TARBALL_FILES ${CPPCHECK_DIRECTORY})

endif()

#
# shellcheck
#
if (SHELLCHECK)

    set(SHELLCHECK_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shellcheck/output.txt)

    add_shellcheck(
        FILES ../system/scripts/config.sh
        OUTPUT ${SHELLCHECK_OUTPUT}
    )

    add_custom_target(shellcheck DEPENDS ${SHELLCHECK_OUTPUT})
    add_dependencies(tarball shellcheck)
    append(TARBALL_FILES ${SHELLCHECK_OUTPUT})

endif()

