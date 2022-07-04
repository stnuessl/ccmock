=========================
ccmock (Work in Progress)
=========================

.. image:: https://github.com/stnuessl/ccmock/actions/workflows/main.yaml/badge.svg
   :alt: CI
   :target: https://github.com/stnuessl/ccmock/actions

Automatic mock creation for unit tests in C and C++ using Google's 
`gtest and gmock <https://google.github.io/googletest/>`_.

.. contents::

About
=====

Motivation
----------

Advantages
----------

Disadvantages
-------------

**ccmock** either needs a 
`compilation database 
<https://clang.llvm.org/docs/JSONCompilationDatabase.html>`_ 
or a `compile_flags.txt
<https://clang.llvm.org/docs/JSONCompilationDatabase.html#alternatives>`_
to be able to parse a project's source files.

Features
========

Installation
============

Supported Platforms
-------------------

* `Arch Linux <https://archlinux.org/>`_
* `Fedora <https://getfedora.org/>`_
* `Ubuntu <https://ubuntu.com/>`_

Required Programs
-----------------

* `bash <https://www.gnu.org/software/bash/bash.html>`_
* `gcc 12.1.0 <https://gcc.gnu.org/>`_  
* `clang 14.0.0 <https://clang.llvm.org/>`_
* `git <https://git-scm.com/>`_
* `make <https://www.gnu.org/software/make/>`_
* `util-linux <https://github.com/util-linux/util-linux>`_
* `find-utils <https://www.gnu.org/software/findutils/>`_

Dependencies
------------

* `llvm >= 14.0.0 <https://llvm.org/>`_
* `clang >= 14.0.0 <https://clang.llvm.org/>`_


Procedure
---------

Install all required tools and libraries:

.. code:: sh

   pacman -Syu llvm clang util-linux find-utils bash gcc clang git make \
   util-linux find-utils

.. code:: sh

   apt-get install llvm llvm-dev clang libclang-dev util-linux find utils bash \
   gcc g++ clang git make util-linux 


Configure the build system:

.. code:: sh

   bash ./configure.sh


Build the project with clang:

.. code:: sh

   make 

If you want to build the project with gcc, run:

.. code:: sh

   make CXX=gcc LD=gcc

Install the built program to */usr/local/bin*:

.. code:: sh

  make install


Usage
=====

Examples
--------

Write generated mocks for *source.c* to *source-mocks.hpp*.

.. code:: sh

   ccmock -o source-mocks.inc source.c

Write generated mocks to stdout.

.. code:: sh

   ccmock source.cpp 

Usually _ccmock_ will need additional information to be able to parse the
input file. Use **ccmock** with a compilation database:

.. code:: sh

   ccmock --compile-commands=compile_commands.json source.cpp

Use _ccmock_ with configuration files:

.. code:: sh

   ccmock --config=ccmock.yaml,source.yaml source.cpp


Open Points
===========

* Mocking C++ is not implemented
* The structure of the configuration file potentially needs reworked

