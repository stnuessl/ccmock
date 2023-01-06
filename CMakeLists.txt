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

cmake_minimum_required(VERSION 3.24)

include(cmake/util.cmake)

# Global hardcoded options
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_DEPFILE_EXTENSION_REPLACE ON)

# Make release the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release CACHE STRING "")
endif()

# Disable cmake default flags
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")

# Ensure that there really is only just one timestamp 
# throughout the whole build.
if (NOT DEFINED ENV{SOURCE_DATE_EPOCH})
    string(TIMESTAMP UNIX_TIME "%s" UTC)
    set(ENV{SOURCE_DATE_EPOCH} ${UNIX_TIME})
endif()

project(
    ccmock
    VERSION 0.0.1
    LANGUAGES CXX
)

set(COMPILATION_DATABASE ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json)


#
# Detect library information.
#
execute_process(
    COMMAND llvm-config --includedir
    OUTPUT_VARIABLE LLVM_INCLUDE_DIRECTORY
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
    COMMAND llvm-config --libdir
    OUTPUT_VARIABLE LLVM_LIBRARY_DIRECTORY
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

find_library(
    LIB_LLVM
    NAMES LLVM
    PATHS ${LLVM_LIBRARY_DIRECTORY}
    REQUIRED
)

find_library(
    LIB_CLANG
    NAMES clang-cpp
    PATHS ${LLVM_LIBRARY_DIRECTORY}
    REQUIRED
)

#
# Configure ccmock target
#
add_executable(
    ${CMAKE_PROJECT_NAME}
    src/FFF.cpp
    src/MockAction.cpp
    src/CMocka.cpp
    src/CompilationDatabase.cpp
    src/util/Decl.cpp
    src/Config.cpp
    src/Generator.cpp
    src/main.cpp
    src/ASTVisitor.cpp
    src/GMock.cpp
    src/ActionFactory.cpp
)

set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES EXPORT_COMPILE_COMMANDS ON
               CXX_STANDARD 20
               CXX_STANDARD_REQUIRED ON
               INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
               INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

set_property(
    TARGET ${CMAKE_PROJECT_NAME}
    PROPERTY COMPILE_DEFINITIONS
)

set_property(
    TARGET ${CMAKE_PROJECT_NAME}
    PROPERTY COMPILE_OPTIONS 
)

set_property(
    TARGET ${CMAKE_PROJECT_NAME}
    PROPERTY LINK_OPTIONS 
)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE src
            ${LLVM_INCLUDE_DIRECTORY}
)

target_compile_definitions(
    ${CMAKE_PROJECT_NAME}
    PRIVATE _GNU_SOURCE
            CCMOCK_VERSION_CORE=\"${CMAKE_PROJECT_VERSION}\"
            CCMOCK_WEBSITE=\"https://github.com/stnuessl/ccmock\"
            $<$<CONFIG:RELEASE>:NDEBUG>
            ${EXTRA_CPPFLAGS}
)

target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE -fno-exceptions 
            -fno-plt
            -fno-rtti
            -fstack-protector-strong
            -mtune=generic
            -fPIC
            $<$<CONFIG:DEBUG>:-Wall>
            $<$<CONFIG:DEBUG>:-Wextra>
            $<$<CONFIG:DEBUG>:-Wpedantic>
            $<$<CONFIG:DEBUG>:-Og>
            $<$<CONFIG:DEBUG>:-g2>
            $<$<CONFIG:RELEASE>:-O2>
            $<$<CONFIG:RELEASE>:-fdata-sections>
            $<$<CONFIG:RELEASE>:-ffunction-sections>
            ${EXTRA_CXXFLAGS}
)

target_link_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE -fPIC
            $<$<CONFIG:RELEASE>:-O2>
            $<$<CONFIG:RELEASE>:-Wl,--gc-sections>
            ${EXTRA_LDFLAGS}
)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE ${LIB_LLVM}
            ${LIB_CLANG}
)

install(TARGETS ${CMAKE_PROJECT_NAME})

if(${CCMOCK_REQUIRE_DEPS})
    set(CCMOCK_REQUIRE_DEPS REQUIRED)
endif()

find_program(FIND find ${CCMOCK_REQUIRE_DEPS})
find_program(TAR tar ${CCMOCK_REQUIRE_DEPS})

if(FIND AND TAR)

    set(CCMOCK_BIN ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME})
    set(TARBALL_FILES CACHE INTERNAL "")

    append(TARBALL_FILES ${CCMOCK_BIN})

    add_custom_target(tarball)

    add_subdirectory(test)

    include(cmake/ci.cmake)
endif()

