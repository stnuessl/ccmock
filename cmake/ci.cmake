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


