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

function(add_cppcheck)

    set(FLAGS "")
    set(
        ONE_VALUE_ARGS 
        TARGET 
        COMPILATION_DATABASE
        CACHE_DIRECTORY
        OUTPUT_DIRECTORY
        OUTPUT_FILE
    )
    set(MULTI_VALUE_ARGS "")

    cmake_parse_arguments(
        ARG
        "${FLAGS}"
        "${ONE_VALUE_ARGS}"
        "${MULTI_VALUE_ARGS}"
        ${ARGN}
    )

    if (NOT DEFINED ARG_TARGET)
        message(
            FATAL_ERROR 
            "${CMAKE_CURRENT_FUNCTION}: argument \"TARGET\" not provided."
        )
    endif()

    if (NOT DEFINED ARG_COMPILATION_DATABASE)
        message(
            FATAL_ERROR
            "${CMAKE_CURRENT_FUNCTION}: " 
            "argument \"COMPILATION_DATABASE\" not provided."
        )
    endif()

    if (NOT DEFINED ARG_CACHE_DIRECTORY)
        message(
            FATAL_ERROR 
            "${CMAKE_CURRENT_FUNCTION}: "
            "argument \"OUTPUT_DIRECTORY\" not provided."
        )
    endif()

    if (NOT DEFINED ARG_OUTPUT_DIRECTORY)
        message(
            FATAL_ERROR 
            "${CMAKE_CURRENT_FUNCTION}: "
            "argument \"OUTPUT_DIRECTORY\" not provided."
        )
    endif()

    if (NOT DEFINED ARG_OUTPUT_FILE)
        message(
            FATAL_ERROR
            "${CMAKE_CURRENT_FUNCTION}: argument \"OUTPUT_FILE\" not provided."
        )
    endif()

    get_target_property(TARGET_SOURCE_DIRECTORY ${ARG_TARGET} SOURCE_DIR)
    get_target_property(TARGET_SOURCE_FILES ${ARG_TARGET} SOURCES)

    foreach(SOURCE_FILE ${TARGET_SOURCE_FILES})
        list(APPEND SOURCE_FILES ${TARGET_SOURCE_DIRECTORY}/${SOURCE_FILE})
    endforeach()

    set(CPPCHECK_RESULT ${ARG_OUTPUT_DIRECTORY}/cppcheck.xml)
    
    file(MAKE_DIRECTORY ${ARG_CACHE_DIRECTORY})
    file(MAKE_DIRECTORY ${ARG_OUTPUT_DIRECTORY})

    add_custom_command(
        OUTPUT ${CPPCHECK_RESULT}
        COMMAND cppcheck 
                --cppcheck-build-dir=${ARG_CACHE_DIRECTORY}
                --enable=all
                --inconclusive
                --inline-suppr
                --library=posix
                --platform=native
                --suppress=allocaCalled
                --suppress=missingInclude
                --suppress=readdirCalled
                --suppress=unusedFunction
                --template=gcc
                --quiet
                --xml
                --project=${ARG_COMPILATION_DATABASE}
                --output-file=${CPPCHECK_RESULT}
        DEPENDS ${ARG_COMPILATION_DATABASE}
                ${SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    set(CPPCHECK_HTML_INDEX ${ARG_OUTPUT_DIRECTORY}/index.html)

    add_custom_command(
        OUTPUT ${CPPCHECK_HTML_INDEX}
        COMMAND cppcheck-htmlreport 
                --file=${CPPCHECK_RESULT}
                --title=${ARG_TARGET}
                --report-dir=${ARG_OUTPUT_DIRECTORY}
        DEPENDS ${CPPCHECK_RESULT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    set(${ARG_OUTPUT_FILE} ${CPPCHECK_HTML_INDEX} PARENT_SCOPE)
endfunction(add_cppcheck)
