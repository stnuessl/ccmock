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

function(add_clang_analysis)

    set(FLAGS "")
    set(
        ONE_VALUE_ARGS 
        TARGET 
        COMPILATION_DATABASE
        CACHE_DIRECTORY
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

    if (NOT DEFINED ARG_OUTPUT_FILES)
        message(
            FATAL_ERROR
            "${CMAKE_CURRENT_FUNCTION}: argument \"OUTPUT_FILES\" not provided."
        )
    endif()

    # Retrieve required properties of the target which shall be analyzed
    get_target_property(TARGET_SOURCE_DIRECTORY ${ARG_TARGET} SOURCE_DIR)
    get_target_property(TARGET_SOURCE_FILES ${ARG_TARGET} SOURCES)
    get_target_property(TARGET_CXX_STANDARD ${ARG_TARGET} CXX_STANDARD)
    get_target_property(COMPILE_DEFS ${ARG_TARGET} COMPILE_DEFINITIONS)
    get_target_property(INCLUDE_DIRS ${ARG_TARGET} INCLUDE_DIRECTORIES)
    get_target_property(COMPILE_OPTS ${ARG_TARGET} COMPILE_OPTIONS)

    foreach(SOURCE_FILE ${TARGET_SOURCE_FILES})
        cmake_path(
            ABSOLUTE_PATH SOURCE_FILE
            BASE_DIRECTORY ${TARGET_SOURCE_DIRECTORY}
            NORMALIZE
        )

        list(APPEND SOURCE_FILES ${SOURCE_FILE})
    endforeach()

    # Transform these variables with generator expressions suitable for use
    # for command-line invocations.
    set(COMPILE_DEFS $<$<BOOL:${COMPILE_DEFS}>:-D$<JOIN:${COMPILE_DEFS},;-D>>)
    set(INCLUDE_DIRS $<$<BOOL:${INCLUDE_DIRS}>:-I$<JOIN:${INCLUDE_DIRS},;-I>>)
    set(COMPILE_OPTS $<FILTER:${COMPILE_OPTS},EXCLUDE,"^[ \t\r\n]*$">)

    # Generate a abstract syntax tree file for every translation unit.
    foreach(SOURCE_FILE ${SOURCE_FILES})
        cmake_path(
            RELATIVE_PATH SOURCE_FILE
            BASE_DIRECTORY ${TARGET_SOURCE_DIRECTORY}
            OUTPUT_VARIABLE RELATIVE_PATH
        )

        cmake_path(REMOVE_EXTENSION RELATIVE_PATH)

        set(AST_FILE ${ARG_CACHE_DIRECTORY}/${RELATIVE_PATH}.ast)

        cmake_path(GET AST_FILE PARENT_PATH AST_FILE_DIRECTORY)
        file(MAKE_DIRECTORY ${AST_FILE_DIRECTORY})
        
        add_custom_command(
            OUTPUT ${AST_FILE}
            COMMAND clang
                    -std=c++${TARGET_CXX_STANDARD}
                    "${COMPILE_DEFS}"
                    "${INCLUDE_DIRS}"
                    "${COMPILE_OPTS}"
                    -emit-ast 
                    -o ${AST_FILE}
                    ${SOURCE_FILE}
            DEPENDS ${SOURCE_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
   
        list(APPEND AST_FILES ${AST_FILE})
    endforeach()
 
    # Generate external definitions mapping file referencing the abstract 
    # syntax tress instead of the source files.
    set(CLANG_EXTDEF_MAPPING ${ARG_OUTPUT_DIRECTORY}/externalDefMap.txt)

    file(MAKE_DIRECTORY ${ARG_OUTPUT_DIRECTORY})

    add_custom_command(
        OUTPUT ${CLANG_EXTDEF_MAPPING}
        COMMAND clang-extdef-mapping
                --extra-arg=-Wno-ignored-optimization-argument
                ${AST_FILES}
                > ${CLANG_EXTDEF_MAPPING}
        DEPENDS ${ARG_COMPILATION_DATABASE}
                ${AST_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    
    list(APPEND ARG_OUTPUT_FILES ${CLANG_EXTDEF_MAPPING})


    # Generate HTML output files for every translation unit.
    foreach(SOURCE_FILE ${SOURCE_FILES})
        cmake_path(GET SOURCE_FILE PARENT_PATH SOURCE_DIRECTORY)

        cmake_path(
            RELATIVE_PATH SOURCE_DIRECTORY
            BASE_DIRECTORY ${TARGET_SOURCE_DIRECTORY}
        )

        # Generate the HTML outputs and a file which is used for correct
        # incremental building.
        cmake_path(GET SOURCE_FILE STEM SOURCE_FILE_STEM)

        set(
            RESULT_DIRECTORY 
            ${ARG_OUTPUT_DIRECTORY}/${SOURCE_DIRECTORY}/${SOURCE_FILE_STEM}
        )

        set(
            OUTPUT_FILE 
            ${ARG_CACHE_DIRECTORY}/${SOURCE_DIRECTORY}/${SOURCE_FILE_STEM}.tag
        )

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND clang 
                    -std=c++${TARGET_CXX_STANDARD}
                    --analyze
                    --analyzer-output html
                    --output ${RESULT_DIRECTORY}
                    -Xclang -analyzer-config 
                    -Xclang ctu-dir=${ARG_OUTPUT_DIRECTORY}
                    -Xclang -analyzer-config
                    -Xclang experimental-enable-naive-ctu-analysis=true
                    -Xclang -analyzer-config 
                    -Xclang display-ctu-progress=true
                    "${COMPILE_DEFS}"
                    "${INCLUDE_DIRS}"
                    "${COMPILE_OPTS}"
                    ${SOURCE_FILE}
                    && touch ${OUTPUT_FILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${CLANG_EXTDEF_MAPPING} ${SOURCE_FILE}
            COMMAND_EXPAND_LISTS
            VERBATIM
        )

        list(APPEND ARG_OUTPUT_FILES ${OUTPUT_FILE})
    endforeach()

    # Publish generated outputs to the caller of this function.
    set(${ARG_OUTPUT_FILES} PARENT_SCOPE)
endfunction(add_clang_analysis)
