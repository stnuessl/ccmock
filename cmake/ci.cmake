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

set(TARBALL_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}.tar.gz)
set(TARBALL ${CMAKE_CURRENT_BINARY_DIR}/${TARBALL_NAME})

#
# Custom tool invocations for continuous integration builds
#
set(ENV_FILE ${CMAKE_CURRENT_BINARY_DIR}/env.txt)

execute_process(
    OUTPUT_FILE ${ENV_FILE}
    COMMAND env 
            ARTIFACTORY_API_KEY= 
            DOCKER_USERNAME=
            DOCKER_PASSWORD=
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)

append(TARBALL_FILES ${ENV_FILE})

foreach(DEP_FILE ${TARBALL_FILES})
    list(
        APPEND TARBALL_INPUT
        $<PATH:RELATIVE_PATH,${DEP_FILE},${CMAKE_CURRENT_BINARY_DIR}>
    )
endforeach()

add_custom_command(
    TARGET tarball
    BYPRODUCTS ${TARBALL}
    COMMAND find ${TARBALL_INPUT} -type f -size +0
            | tar 
            --create
            --file ${TARBALL}
            --gzip
            --directory ${CMAKE_CURRENT_BINARY_DIR}
            --files-from=-
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    POST_BUILD
    VERBATIM
)

cmake_host_system_information(
    RESULT OS_RELEASE 
    QUERY OS_RELEASE
)

cmake_host_system_information(
    RESULT DISTRIBUTION_NAME
    QUERY DISTRIB_ID
)

string(TIMESTAMP DATE "%Y-%m-%d" UTC)
string(TIMESTAMP TIME "%H:%M:%S" UTC)

string(RANDOM LENGTH 20 ALPHABET 0123456789abcdef BUILD_ID)

add_custom_target(
    artifactory-upload
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/cmake/artifactory.py
            --repository-url https://nuessle.jfrog.io/artifactory/ccmock-local
            --repository-path ${DATE}/${TIME}/${BUILD_ID}
            --file ${TARBALL}
            --properties 
                action=$ENV{GITHUB_RUN_ID}
                branch=$ENV{GITHUB_REF_NAME}
                cmake=${CMAKE_VERSION}
                commit=$ENV{GITHUB_SHA}
                compiler=${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}
                date=${DATE}
                distribution=${DISTRIBUTION_NAME}
                id=${BUILD_ID}
                job=$ENV{GITHUB_JOB}
                kernel=${OS_RELEASE}
                os=${CMAKE_SYSTEM_NAME}
                platform=${CMAKE_SYSTEM_PROCESSOR}
                source_date_epoch=$ENV{SOURCE_DATE_EPOCH}
                time=${TIME}
                timezone=utc
                ${CMAKE_PROJECT_NAME}=${CMAKE_PROJECT_VERSION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/artifactory.py
            ${TARBALL}
    VERBATIM
)


