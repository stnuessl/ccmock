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

CC		:= gcc
CXX		:= gcc
LD		:= gcc
CCMOCK	:= ./../../../build/release/ccmock

Q	:= @

srcdir		:= src
testdir		:= tests
BUILD_DIR	:= build

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
	-I$(BUILD_DIR)/ \
	-I$(srcdir)/

CFLAGS		:= -std=c99 -O2 $(shell pkgconf --cflags gmock gtest)
CXXFLAGS	:= $(shell pkgconf --cflags gmock gtest)
LDFLAGS		:= -lstdc++ $(shell pkgconf --libs gmock gtest)

ut_src		:= $(shell find $(testdir) -name "*.cpp")
ut_reports	:= $(patsubst %.cpp,%.xml,$(ut_src))

# Needed to automate cloning of the repository
src			:= $(patsubst $(testdir)/%.cpp,$(srcdir)/%.c,$(ut_src))

# Disable all implicit rules
.SUFFIXES:

# Keep intermediate build files
.SECONDARY:

all: $(ut_reports)

$(testdir)/%.xml: $(testdir)/%.out
	@printf "  %-6s %s\n" "RUN" "$@"
	$(Q)./$< --gtest_output=xml:$@ > /dev/null

$(testdir)/%.out: $(srcdir)/%.o $(testdir)/%.o
	@printf "  %-6s %s\n" "LD" "$@"
	$(Q)$(LD) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(testdir)/%.o: $(testdir)/%.cpp $(testdir)/%.inc
	@printf "  %-6s %s\n" "CXX" "$@"
	$(Q)$(CC) -c -o $@ $< $(CPPFLAGS) $(CXXFLAGS)

$(srcdir)/%.o: $(srcdir)/%.c $(BUILD_DIR)/compile_commands.json
	@printf "  %-6s %s\n" "CC" "$@"
	$(Q)$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

$(src): $(srcdir)

$(testdir)/%.inc: $(srcdir)/%.c $(BUILD_DIR)/compile_commands.json ccmock.yaml
	@printf "  %-6s %s\n" "MOCK" "$@"
	$(Q)$(CCMOCK) \
		-o $@ \
		--config=ccmock.yaml \
		--compile-commands=$(BUILD_DIR)/compile_commands.json \
		$<

$(BUILD_DIR)/compile_commands.json: $(srcdir)
	@printf "  %-6s %s\n" "CONF" "$@"
	$(Q)cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S $(srcdir) -B $(BUILD_DIR) \
		> /dev/null

$(srcdir):
	@printf "  %-6s %s\n" "CLONE" "$@"
	@git clone --quiet https://github.com/zlib-ng/zlib-ng $@
	@cd $@ && git checkout tags/2.0.6 -b ccmock_test > /dev/null 2>&1

clean:
	rm -rf \
		$(testdir)/*.inc \
		$(testdir)/*.o \
		$(testdir)/*.out \
		$(testdir)/*.xml \
		$(srcdir)/*.o \
		$(BUILD_DIR)

