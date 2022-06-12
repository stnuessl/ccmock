#
# The MIT License (MIT)
#
# Copyright (c) 2021  Steffen Nuessle
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

Q :=

SHELL := bash -o pipefail

makefile := $(lastword $(MAKEFILE_LIST))
makedir := $(patsubst %/,%,$(dir $(makefile)))

DOCKER_DIR := build/docker
TAG_PREFIX := crudebox

targets := \
	archlinux-build \
#	fedora-build \
#	ubuntu-build

#SCAN_DIR			:= $(DOCKER_DIR)/scan
#SCAN_ARCH_BUILD		:= $(SCAN_DIR)/archlinux-build.txt
#SCAN_FEDORA_BUILD	:= $(SCAN_DIR)/fedora-build.txt
#SCAN_UBUNTU_BUILD	:= $(SCAN_DIR)/ubuntu-build.txt
#
#DOCKER_SCAN_FLAGS := \
#	--token 3526b487-387e-42e9-8993-934f9621185c \
#	--accept-license \
#	--severity low

dirs := \
	$(DOCKER_DIR) \
	$(SCAN_DIR)

all: $(patsubst %,$(DOCKER_DIR)/%.ack,$(targets))

%: $(DOCKER_DIR)/%.ack
	$(Q)docker run \
		--interactive \
		--tty=$(if $(CMD),false,true) \
		--rm \
		--env XDG_SESSION_TYPE \
		--env ARTIFACTORY_API_KEY \
		--volume ${PWD}:/crudebox \
		--user $$(id -u):$$(id -g) \
		--workdir /crudebox \
		$(TAG_PREFIX):$* $(CMD)

$(DOCKER_DIR)/%.ack: $(makedir)/%/Dockerfile | $(dirs)
	$(Q)docker build --tag $(TAG_PREFIX):$* $(^D)
	@touch $@

docker-clean:
	docker image ls \
		| grep "$(TAG_PREFIX)" \
		| awk '{ print $$3 }' \
		| xargs --no-run-if-empty docker image rm --force
	rm -rf $(DOCKER_DIR)

$(dirs):
	mkdir -p $@

.PHONY: \
	all \
	docker-clean

.SILENT: \
	$(dirs) \
	docker-clean

