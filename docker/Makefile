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

Q :=

SHELL := bash -o pipefail

makefile := $(lastword $(MAKEFILE_LIST))
makedir := $(patsubst %/,%,$(dir $(makefile)))

PROJECT_NAME = ccmock

DOCKER_DIR := build/docker
TAG_PREFIX := $(PROJECT_NAME)

targets := \
	archlinux-build \
	fedora-build \
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

#
# Disable automatic deletion of intermediate files
#
.SECONDARY:

all: $(patsubst %,$(DOCKER_DIR)/%.ack,$(targets))

%: $(DOCKER_DIR)/%.ack
	$(Q)docker run \
		--interactive \
		--tty=$(if $(CMD),false,true) \
		--rm \
		--env ARTIFACTORY_API_KEY \
		--env CI \
		--env GITHUB_JOB \
		--env GITHUB_REF_NAME \
		--env GITHUB_RUN_ID \
		--env GITHUB_SHA \
		--env SOURCE_DATE_EPOCH \
		--volume ${PWD}:/project/$(PROJECT_NAME) \
		--user $$(id -u):$$(id -g) \
		--workdir /project/$(PROJECT_NAME) \
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

