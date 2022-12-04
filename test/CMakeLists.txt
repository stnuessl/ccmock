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

find_library(LIB_GTEST NAMES gtest ${CCMOCK_REQUIRE_DEPS})
find_library(LIB_GMOCK NAMES gmock ${CCMOCK_REQUIRE_DEPS})
find_library(LIB_CMOCKA NAMES cmocka ${CCMOCK_REQUIRE_DEPS})

find_program(PKGCONF pkgconf ${CCMOCK_REQUIRE_DEPS})

add_subdirectory(static)

if (LIB_GTEST AND LIB_GMOCK AND LIB_CMOCKA AND PKGCONF)

    execute_process(
        COMMAND pkgconf --cflags gtest
        OUTPUT_VARIABLE GTEST_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )

    add_subdirectory(system)
    add_subdirectory(unit)

endif()
