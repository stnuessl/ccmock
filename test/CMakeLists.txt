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

find_library(LIB_GTEST NAMES gtest ${CCMOCK_REQUIRE_DEPS})
find_library(LIB_GMOCK NAMES gmock ${CCMOCK_REQUIRE_DEPS})
find_library(LIB_CMOCKA NAMES cmocka ${CCMOCK_REQUIRE_DEPS})

find_program(PKGCONF pkgconf ${CCMOCK_REQUIRE_DEPS})

add_subdirectory(static)

if (LIB_GTEST AND LIB_GMOCK AND LIB_CMOCKA AND PKGCONF)

    execute_process(
        COMMAND pkgconf --cflags gtest
        OUTPUT_VARIABLE GTEST_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )

    add_subdirectory(system)
    add_subdirectory(unit)

endif()
