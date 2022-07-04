#!/usr/bin/env bash
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

set -o pipefail

#
# Automatically detect source and header files
#
PROJECT_SRC="$(find src/ -name "*.cpp" -printf "\t%p \\\\\n")"
PROJECT_HDR="$(find src/ -name "*.hpp" -printf "\t%p \\\\\n")"

#
# Define include paths
#
PROJECT_INC="-Isrc/"

INCLUDE_DIR="$(llvm-config --includedir)"

if [[ "/usr/include" != "${INCLUDE_DIR}" ]]; then
    
    # Add as "-isystem" to avoid generation of potential i
    # compiler warnings in library headers.
    PROJECT_INC="${PROJECT_INC} '-isystem ${INCLUDE_DIR}'"
fi

#
# libclang-cpp.so seems to be more special as not every distribution seems
# to provide the symlink.
#
PROJECT_LDLIBS="$(llvm-config --libs)"
LIBDIR="$(llvm-config --libdir)"
VERSION="$(llvm-config --version | sed -n 's/\([0-9]\+\).*$/\1/p')"

if [[ -f "${LIBDIR}/libclang-cpp.so" ]]; then
    PROJECT_LDLIBS="${PROJECT_LDLIBS} -lclang-cpp"
elif [[ -f "${LIBDIR}/libclang-cpp.so.${VERSION}" ]]; then
    PROJECT_LDLIBS="${PROJECT_LDLIBS} -l:libclang-cpp.so.${VERSION}"
else
    echo "error: libclang-cpp.so: failed to detect shared library"
    exit 1;
fi


#
# Add additional linker flags required to find shared libraries
#
if [[ ${LIBDIR} != '/usr/lib' ]]; then
    PROJECT_LDFLAGS="-L${LIBDIR}"
else
    PROJECT_LDFLAGS=""
fi


#
# Ensure that arguments are nicely formatted
#
PROJECT_INC="$(echo "${PROJECT_INC}" | xargs printf "\t%s \\\\\n")"
PROJECT_LDLIBS="$(echo "${PROJECT_LDLIBS}" | xargs printf "\t%s \\\\\n")"
PROJECT_LDFLAGS="$(echo "${PROJECT_LDFLAGS}" | xargs printf "\t%s \\\\\n")"


#
# Define appropriate environment variables for envsubst
# 
export PROJECT_SRC
export PROJECT_HDR
export PROJECT_INC
export PROJECT_LDLIBS
export PROJECT_LDFLAGS

#
# Generate the project's Makefile
#

# shellcheck disable=SC2312
mapfile -t VARLIST < <(env | grep PROJECT | cut -d "=" -f 1)

VARS="$(printf "\${%s} " "${VARLIST[@]}")"
envsubst "${VARS}" < Makefile.in > Makefile || (rm -f Makefile && false)
