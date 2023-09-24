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

function(add_clang_tidy)
    set(FLAGS 
        WARNINGS_AS_ERRORS
        VERBOSE 
    )
    set(
        ONE_VALUE_ARGS 
        TARGET
        COMPILATION_DATABASE
        VIRTUAL_FILESYSTEM
        CONFIG_FILE
        OUTPUT_DIRECTORY 
        OUTPUT_FILES
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

    if (DEFINED ARG_VIRTUAL_FILESYSTEM)
        set(CLANG_TIDY_VFS_ARG --vfsoverlay=${ARG_VIRITUAL_FILESYSTEM})
    endif()

    if (DEFINED ARG_CONFIG_FILE)
        set(CLANG_TIDY_CONFIG_FILE_ARG --config-file=${ARG_CONFIG_FILE})
    endif()

    if (NOT DEFINED ARG_OUTPUT_DIRECTORY)
        message(
            FATAL_ERROR 
            "${CMAKE_CURRENT_FUNCTION}: "
            "argument \"OUTPUT_DIRECTORY\" not provided."
        )
    endif()

    if (ARG_WARNINGS_AS_ERRORS)
        set(CLANG_TIDY_WARNINGS_AS_ERRORS --warnings-as-errors=*)
    endif()

    if (ARG_VERBOSE)
        set(COMMAND_REDIRECT | tee)
    else()
        set(CLANG_TIDY_QUIET --quiet)
        set(COMMAND_REDIRECT >)
    endif()

    get_target_property(TARGET_SOURCE_DIRECTORY ${ARG_TARGET} SOURCE_DIR)
    get_target_property(TARGET_SOURCE_FILES ${ARG_TARGET} SOURCES)

    foreach(SOURCE_FILE ${TARGET_SOURCE_FILES})
        cmake_path(
            REPLACE_EXTENSION SOURCE_FILE
            LAST_ONLY .txt
            OUTPUT_VARIABLE OUTPUT_FILE
        )

        set(OUTPUT_FILE ${ARG_OUTPUT_DIRECTORY}/${OUTPUT_FILE})

        cmake_path(
            ABSOLUTE_PATH SOURCE_FILE
            BASE_DIRECTORY ${TARGET_SOURCE_DIRECTORY}
            NORMALIZE
        )
    
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND clang-tidy
                    -p ${ARG_COMPILATION_DATABASE}
                    ${CLANG_TIDY_CONFIG_FILE_ARG}
                    ${CLANG_TIDY_VFS_ARG}
                    ${CLANG_TIDY_WARNINGS_AS_ERRORS}
                    ${CLANG_TIDY_QUIET}
                    ${SOURCE_FILE}
                    ${COMMAND_REDIRECT} ${OUTPUT_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${ARG_COMPILATION_DATABASE}
                    ${ARG_CONFIG_FILE}
                    ${SOURCE_FILE}
            VERBATIM
        )

        list(APPEND ARG_OUTPUT_FILES ${OUTPUT_FILE})
    endforeach()

    set(${ARG_OUTPUT_FILES} PARENT_SCOPE)
endfunction(add_clang_tidy)
