#
# Dockerfile
#
# Used to create an image containing archlinux
# with all required dependencies to build the project.
#

FROM archlinux:base
RUN pacman -Syu --noconfirm \
    clang \
    cppcheck \
    ctags \
    findutils \
    gcc \
    lcov \
    llvm \
    make \
    pkgconfig \
    util-linux \
&& pacman -Scc --noconfirm \
&& useradd \
    --shell /usr/bin/bash \
    --no-create-home \
    arch
USER arch
CMD [ "/usr/bin/bash" ]

