
# Dockerfile
#
# Used to create an image containing Fedora
# with all required dependencies to build the project.
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

