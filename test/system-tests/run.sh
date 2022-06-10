#!/usr/bin/bash env
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


set -e

./build/release/ccmock \
    --compile-commands=test/system-tests/compile-commands.json \
    --config=test/system-tests/simple.yaml \
    test/system-tests/simple.c \
    > test/system-tests/simple-test.inc


clang \
    -c \
    $(pkg-config --cflags gmock gtest) \
    -fno-rtti \
    -o test/system-tests/simple-test.o \
    test/system-tests/simple-test.cpp

clang \
    -c \
    -o test/system-tests/simple.o \
    test/system-tests/simple.c

clang \
    -lstdc++ \
    $(pkg-config --libs gmock gtest) \
    test/system-tests/simple.o \
    test/system-tests/simple-test.o \
    -o test/system-tests/simple.elf

test/system-tests/simple.elf

rm -f test/system-tests/simple-test.inc \
    test/system-tests/simple-test.o \
    test/system-tests/simple.o \
    test/system-tests/simple.elf
