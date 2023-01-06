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

FROM fedora:latest
RUN dnf install --assumeyes \
    clang \
    clang-devel \
    clang-tools-extra \
    cmake \
    cppcheck \
    cppcheck-htmlreport \
    ctags \
    findutils \
    gcc \
    gettext \
    git \
    lcov \
    llvm-devel \
    libcmocka-devel \
    gtest-devel \
    gmock-devel \
    make \
    pkgconfig \
    util-linux \
&& dnf clean dbcache --assumeyes \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    fedora
USER fedora
CMD [ "/usr/bin/bash" ]

