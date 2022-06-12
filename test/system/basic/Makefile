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

SHELL	:= bash -o pipefail
CC		:= clang
CXX		:= clang
LD		:= clang

src		:= $(shell find src -name "*.c")
obj		:= $(patsubst %.c,%.o,$(src))
ut_src	:= $(shell find test -name "*.cpp")
ut_obj	:= $(patsubst %.cpp,%.o,$(ut_src))
ut_gen	:= $(patsubst src/%.c,test/%.inc,$(src))
ut_bin	:= $(patsubst %.cpp,%.elf,$(ut_src))
ut_out	:= $(patsubst %.cpp,%.txt,$(ut_src))

CPPFLAGS	:= $(shell pkg-config --cflags gmock gtest)
CFLAGS		:=
CXXFLAGS	:= -std=c++11 -fno-rtti
LDLIBS		:= -lstdc++ $(shell pkg-config --libs gmock gtest)

#
# Disable all implicit rules
#
.SUFFIXES:

#
# Prevent make from automatically removing
# intermediate files when invoking "all".
#
.SECONDARY:


all: tests

tests: $(ut_out)

%.txt: %.elf
	./$< | tee $@

test/%.elf: test/%.o src/%.o
	$(LD) -o $@ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) $^

src/%.o: src/%.c
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

test/%.o: test/%.cpp test/%.inc
	$(CXX) -c -o $@ -Isrc $(CPPFLAGS) $(CXXFLAGS) $<

test/%.inc: src/%.c
	./../../../build/release/ccmock -o $@ --config=test/ccmock.yaml $<

clean:
	rm -f $(obj) $(ut_obj) $(ut_bin) $(ut_out) $(ut_gen)

.PHONY: \
	all \
	tests \
	clean


