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

FROM ubuntu:rolling
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get --assume-yes install \
    bsdmainutils \
    clang \
    clang-tools \
    cmake \
    cppcheck \
    curl \
    findutils \
    g++ \
    gcc \
    gettext-base \
    git \
    lcov \
    libclang-dev \
    libcmocka-dev \
    libgmock-dev \
    libgtest-dev \
    llvm-dev \
    make \
    pkgconf \
    universal-ctags \
    uuid-runtime \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    ubuntu
USER ubuntu
CMD [ "/usr/bin/bash" ]

