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
