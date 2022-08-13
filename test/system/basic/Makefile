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
BUILD_DIR := ../../../build/system-tests/basic
RESULT_DIR := ../../../build/system-tests/results/basic

export PROJECT_DIRECTORY := $(CURDIR)

SHELL	:= bash -o pipefail
CC		:= clang
CXX		:= clang
LD		:= clang

Q :=

srcdir := src
ut_srcdir := $(CCMOCK_BACKEND)

outdir := $(BUILD_DIR)/$(srcdir)
ut_outdir := $(BUILD_DIR)/$(ut_srcdir)

src		:= $(shell find $(srcdir) -name "*.c")
objs	:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(src))

ifeq ($(CCMOCK_BACKEND),cmocka)
ut_src	:= $(shell find $(ut_srcdir) -name "*.c")
ut_objs	:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(ut_src))
ut_gen	:= $(patsubst %.c,$(RESULT_DIR)/%.inc,$(ut_src))
ut_out	:= $(patsubst %.c,$(RESULT_DIR)/%.xml,$(ut_src))
else
ut_src	:= $(shell find $(ut_srcdir) -name "*.cpp")
ut_objs	:= $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(ut_src))
ut_gen	:= $(patsubst %.cpp,$(RESULT_DIR)/%.inc,$(ut_src))
ut_out	:= $(patsubst %.cpp,$(RESULT_DIR)/%.xml,$(ut_src))
endif

compile_commands := $(BUILD_DIR)/compile_commands.json


#
# Define include directories
#
ut_inc := \
	-I$(srcdir) \
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


CFLAGS		:= -Wall -std=c99 -O2
CXXFLAGS	:= -Wall -std=c++17 -O2

ifeq ($(CCMOCK_BACKEND),gmock)
CPPFLAGS	:= $(shell pkgconf --cflags gmock gtest)
LDFLAGS		:= -lstdc++ $(shell pkgconf --libs gmock gtest)
else ifeq ($(CCMOCK_BACKEND),fff)
CPPFLAGS	:= $(shell pkgconf --cflags gtest)
LDFLAGS		:= -lstdc++ $(shell pkgconf --libs gtest)
else ifeq ($(CCMOCK_BACKEND),cmocka)
CPPFLAGS	:= $(shell pkgconf --cflags cmocka)
LDFLAGS		:= -lstdc++ $(shell pkgconf --libs cmocka)
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

#
# Disable all implicit rules
#
.SUFFIXES:


#
# Prevent make from automatically removing
# intermediate files when invoking "all".
#
.SECONDARY:


all: $(ut_out)

$(RESULT_DIR)/%.xml: $(BUILD_DIR)/%.out
	@printf "$(green)Executing [ $< ]$(reset)\n"
ifeq ($(CCMOCK_BACKEND),cmocka)
ifeq ($(Q),@)
	$(Q)CMOCKA_MESSAGE_OUTPUT=xml CMOCKA_XML_FILE=$@ $< 
else
	$(Q)CMOCKA_MESSAGE_OUTPUT=xml $< | tee $@ || (rm -f $@ && false)
endif
else ifeq ($(Q),@)
	$(Q)$< --gtest_output=xml:$@ > /dev/null 2>&1
else
	$(Q)$< --gtest_output=xml:$@
endif

$(ut_outdir)/%.out: $(ut_outdir)/%.o $(outdir)/%.o
	@printf "$(yellow)Linking [ $@ ]$(reset)\n"
	$(Q)$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(outdir)/%.o: $(srcdir)/%.c
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $<

$(ut_outdir)/%.o: $(ut_srcdir)/%.c $(RESULT_DIR)/$(CCMOCK_BACKEND)/%.inc
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)$(CXX) -c -o $@ $(ut_inc) $(CPPFLAGS) $(CFLAGS) $<

$(ut_outdir)/%.o: $(ut_srcdir)/%.cpp $(RESULT_DIR)/$(CCMOCK_BACKEND)/%.inc
	@printf "$(blue)Building [ $@ ]$(reset)\n"
	$(Q)$(CXX) -c -o $@ $(ut_inc) $(CPPFLAGS) $(CXXFLAGS) $<

$(RESULT_DIR)/$(CCMOCK_BACKEND)/%.inc: $(srcdir)/%.c $(compile_commands)
	@printf "$(cyan)Generating [ $@ ]$(reset)\n"
	$(Q)$(CCMOCK) \
		-o $@ \
		--compile-commands $(compile_commands) \
		--backend $(CCMOCK_BACKEND) \
		$<

$(compile_commands): compile_commands.in
	@printf "$(magenta)Configuring [ $@ ]$(reset)\n"
	$(Q)envsubst "\$${PROJECT_DIRECTORY}" < $< > $@ || (rm -f $@ && false)

$(ut_out) $(ut_gen) $(objs) $(ut_objs) $(compile_commands): | $(dirs)

$(dirs):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(RESULT_DIR)

.PHONY: \
	all \
	tests \
	clean

.SILENT: \
	$(dirs) \
	clean
