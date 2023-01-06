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

