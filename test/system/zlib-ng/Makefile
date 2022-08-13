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

CCMOCK := ../../../build/release/ccmock
CCMOCK_BACKEND := gmock
BUILD_DIR := ../../../build/system-tests/zlib-ng
RESULT_DIR := ../../../build/system-tests/results/zlib-ng

SHELL	:= bash -o pipefail
CC		:= gcc
CXX		:= gcc
LD		:= gcc

Q :=

srcdir := $(BUILD_DIR)/src
ut_srcdir := $(CCMOCK_BACKEND)

outdir := $(BUILD_DIR)/$(srcdir)
ut_outdir := $(BUILD_DIR)/$(ut_srcdir)

ut_src	:= $(shell find $(ut_srcdir) -name "*.cpp")
# Needed to automate cloning of the repository
src		:= $(patsubst $(ut_srcdir)/%.cpp,$(srcdir)/%.c,$(ut_src))
objs	:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(src))
ut_objs	:= $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(ut_src))
ut_gen	:= $(patsubst %.cpp,$(RESULT_DIR)/%.inc,$(ut_src))
ut_out	:= $(patsubst %.cpp,$(RESULT_DIR)/%.xml,$(ut_src))

compile_commands := $(BUILD_DIR)/build/compile_commands.json


#
# Define include directories
#
ut_inc := \
	-I$(dir $(compile_commands)) \
	$(patsubst %,-I%,$(sort $(dir $(ut_out))))

ifeq ($(CCMOCK_BACKEND),fff)
ut_inc += -I../common/include
endif


#
# Define required directories
#
dirs := \
	$(sort $(dir $(objs))) \
	$(sort $(dir $(ut_objs))) \
	$(sort $(dir $(ut_out)))


# Retrieved from the generated compilation database
CPPFLAGS	:= \
	-DNDEBUG \
	-DHAVE_BUILTIN_CTZ \
	-DHAVE_BUILTIN_CTZLL \
	-DHAVE_POSIX_MEMALIGN \
	-DHAVE_SYS_SDT_H \
	-DHAVE_VISIBILITY_HIDDEN \
	-DHAVE_VISIBILITY_INTERNAL \
	-DUNALIGNED64_OK \
	-DUNALIGNED_OK \
	-DWITH_GZFILEOP \
	-DX86_AVX2 \
	-DX86_AVX2_ADLER32 \
	-DX86_AVX_CHUNKSET \
	-DX86_FEATURES \
	-DX86_PCLMULQDQ_CRC \
	-DX86_SSE2 \
	-DX86_SSE2_CHUNKSET \
	-DX86_SSE2_SLIDEHASH \
	-DX86_SSE42_CMP_STR \
	-DX86_SSE42_CRC_HASH \
	-DX86_SSE42_CRC_INTRIN \
	-DX86_SSSE3 \
	-DX86_SSSE3_ADLER32 \
	-DZLIB_DLL \
	-D_LARGEFILE64_SOURCE=1 \
	-D__USE_LARGEFILE64 \
	$(shell pkgconf --cflags gtest) \
	-I$(BUILD_DIR)/build \
	-I$(srcdir)/

CFLAGS		:= -Wall -std=c99 -O2
CXXFLAGS	:= -Wall -std=c++17 -O2
LDFLAGS		:= -lstdc++

ifeq ($(CCMOCK_BACKEND),gmock)
CFLAGS		+= $(shell pkgconf --cflags gmock gtest)
LDFLAGS		+= $(shell pkgconf --libs gmock gtest)
else
CFLAGS		+= $(shell pkgconf --cflags gtest)
LDFLAGS		+= $(shell pkgconf --libs gtest)
endif


#
# Setting terminal colors
#

ifneq ($(MAKEFILE_COLOR), 0)

red			:= \e[1;31m
green		:= \e[1;32m
yellow		:= \e[1;33m
blue		:= \e[1;34m
magenta		:= \e[1;35m
cyan		:= \e[1;36m
reset		:= \e[0m

endif

# Disable all implicit rules
.SUFFIXES:

# Keep intermediate build files
.SECONDARY:


all: $(ut_out)

$(RESULT_DIR)/%.xml: $(BUILD_DIR)/%.out
	@printf "$(green)Executing [ $< ]$(reset)\n"
ifeq ($(Q),@)
	$(Q)./$< --gtest_output=xml:$@ > /dev/null 2>&1
else
	$(Q)./$< --gtest_output=xml:$@
endif

$(ut_outdir)/%.out: $(ut_outdir)/%.o $(outdir)/%.o
	@printf "$(yellow)Linking [ $@ ]$(reset)\n"
	$(Q)$(LD) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

# As the cmake configuration generates some required header dependencies,
# we use the compilation database to synchronize
$(outdir)/%.o: $(srcdir)/%.c $(compile_commands)
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

$(ut_outdir)/%.o: $(ut_srcdir)/%.cpp $(RESULT_DIR)/$(CCMOCK_BACKEND)/%.inc
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(ut_inc) $(CXXFLAGS) $<

$(RESULT_DIR)/$(CCMOCK_BACKEND)/%.inc: $(srcdir)/%.c $(compile_commands)
	@printf "$(cyan)Generating [ $@ ]$(reset)\n"
	$(Q)$(CCMOCK) \
		-o $@ \
		--compile-commands $(compile_commands) \
		--backend $(CCMOCK_BACKEND) \
		$<

$(src): $(srcdir)/CMakeLists.txt

$(compile_commands): $(srcdir)/CMakeLists.txt
	@printf "$(magenta)Configuring [ $@ ]$(reset)\n"
	$(Q)cmake \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-S $(srcdir) \
		-B $(@D) \
		> /dev/null || (rm -f $@ || false)

$(srcdir)/CMakeLists.txt:
	@printf "$(magenta)Downloading [ zlib-ng ]$(reset)\n"
ifeq ($(Q),@)
	@git clone --quiet https://github.com/zlib-ng/zlib-ng $(srcdir)
	@cd $(srcdir) && git checkout tags/2.0.6 -b ccmock_test > /dev/null 2>&1
else
	@git clone https://github.com/zlib-ng/zlib-ng $(srcdir)
	@cd $(srcdir) && git checkout tags/2.0.6 -b ccmock_test
endif

$(ut_out) $(ut_gen) $(objs) $(ut_objs) $(compile_commands): | $(dirs)

$(dirs):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(RESULT_DIR)

.PHONY: \
	all \
	clean

.SILENT: \
	$(dirs) \
	clean
