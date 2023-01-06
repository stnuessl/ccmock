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

function(add_shellcheck)
    set(FLAGS "")
    set(ONE_VALUE_ARGS OUTPUT)
    set(MULTI_VALUE_ARGS FILES)

    cmake_parse_arguments(
        ARG
        "${FLAGS}"
        "${ONE_VALUE_ARGS}"
        "${MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if (NOT DEFINED ARG_OUTPUT)
        message(
            FATAL_ERROR 
            "${CMAKE_CURRENT_FUNCTION}: argument \"OUTPUT\" not provided."
        )
    endif()

    if (NOT DEFINED ARG_FILES)
        message(
            FATAL_ERROR
            "${CMAKE_CURRENT_FUNCTION}: argument \"FILES\" not provided."
        )
    endif()

    foreach(SCRIPT_FILE ${ARG_FILES})
        cmake_path(ABSOLUTE_PATH SCRIPT_FILE NORMALIZE)

        list(APPEND SHELLCHECK_INPUT ${SCRIPT_FILE})
    endforeach()
 
    cmake_path(GET ARG_OUTPUT PARENT_PATH OUTPUT_DIRECTORY)
    file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})

    add_custom_command(
        OUTPUT ${ARG_OUTPUT}
        COMMAND shellcheck
                --color=auto
                --external-sources
                --format gcc
                --enable all
                --norc
                --shell $<PATH:GET_FILENAME,$ENV{SHELL}>
                ${SHELLCHECK_INPUT}
                | tee ${ARG_OUTPUT} 
                || "(rm -f ${ARG_OUTPUT} && false)"
        DEPENDS ${SHELLCHECK_INPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
endfunction(add_shellcheck)
