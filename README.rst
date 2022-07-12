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
* `gcc <https://gcc.gnu.org/>`_ 12.1.0
* `clang <https://clang.llvm.org/>`_ 14.0.0
* `git <https://git-scm.com/>`_
* `make <https://www.gnu.org/software/make/>`_
* `util-linux <https://github.com/util-linux/util-linux>`_
* `find-utils <https://www.gnu.org/software/findutils/>`_

Dependencies
------------

* `llvm <https://llvm.org/>`_ >= 14.0.0
* `clang <https://clang.llvm.org/>`_ >= 14.0.0


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

General Idea
------------

#. Generate a compilation database *compile_commands.json* for your project.
#. Feed the source file and the compilation database into **ccmock** to create
   mocks for your unit test code.

   ``ccmock --compile-commands=compile_commands.json -o <output> <input>``

#. Use the preprocessor to include the generated output file in your unit test 
   source file.
#. Focus on writing unit test code.
   

Examples
--------

Get Help Message
^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock

.. code:: sh

   ccmock --help

.. code:: sh

   ccmock -h


Write Mock Function to Standard Output
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock <input-file>


Write Mock Function to File
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock -o <output-file> <input-file>


Using Compile Flags
^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock --compile-commands=compile_flags.txt <input-file>
 

Using a Compilation Database
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock --compile-commands=compile_commands.json <input-file>

Specifying Configuration Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock --config=<config>.yaml

.. code:: sh

   ccmock --config=<config>.yaml,<config>.yaml <input-file>

Dump Effective Configuration Settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock --dump-config

.. code:: sh

   ccmock --dump-config -o <output-file>

Open Points
===========

* Mocking C++ is not implemented
* The structure of the configuration file potentially needs reworked

