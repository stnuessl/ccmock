#
# Dockerfile
#
# Used to create an image containing ubuntu linux
# with all required dependencies to build the project.
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

