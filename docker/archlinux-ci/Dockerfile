#
# Dockerfile
#
# Used to create an image containing archlinux
# with all required dependencies to build the project.
#

FROM archlinux:base
RUN pacman -Syu --noconfirm \
    clang \
    cmake \
    cmocka \
    cppcheck \
    findutils \
    gcc \
    git \
    gtest \
    lcov \
    llvm \
    make \
    ninja \
    pkgconf \
    tar \
    shellcheck \
&& pacman -Scc --noconfirm \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    arch
USER arch
CMD [ "/usr/bin/bash" ]

