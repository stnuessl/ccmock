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

find_program(GIT git ${CCMOCK_REQUIRE_DEPS})

add_custom_target(system-tests)
add_dependencies(tarball system-tests)

set(FFF_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/common/include)

add_subdirectory(basic)
add_subdirectory(scripts)

if(GIT)
    add_subdirectory(zlib-ng)
endif()
