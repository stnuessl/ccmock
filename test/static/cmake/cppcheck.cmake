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
                --suppress=missingIncludeSystem
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
