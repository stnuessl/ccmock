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

set(TARGET_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.tag)
set(TARGET_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/config.sh)
set(
    TARGET_DEPS
    ${CMAKE_CURRENT_SOURCE_DIR}/gmock.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/mocking.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/clang.yaml
)

add_custom_command(
    OUTPUT ${TARGET_OUTPUT}
    COMMAND bash config.sh ${CCMOCK_BIN} && touch ${TARGET_OUTPUT}
    DEPENDS config.sh
            ccmock
            gmock.yaml
            mocking.yaml
            clang.yaml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_custom_target(system-tests-scripts-config DEPENDS ${TARGET_OUTPUT})
add_dependencies(system-tests system-tests-scripts-config)
