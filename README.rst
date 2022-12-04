=========================
ccmock (Work in Progress)
=========================

.. image:: https://github.com/stnuessl/ccmock/actions/workflows/build.yaml/badge.svg
   :alt: Build
   :target: https://github.com/stnuessl/ccmock/actions

.. image:: https://github.com/stnuessl/ccmock/actions/workflows/ci.yaml/badge.svg
   :alt: CI
   :target: https://github.com/stnuessl/ccmock/actions

.. image:: https://img.shields.io/badge/License-GPLv3-blue.svg
   :alt: License: GPL v3
   :target: https://www.gnu.org/licenses/gpl-3.0


Automatic mock creation for unit tests in C and C++.

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

* Automatic detection which functions need to be mocked.
* Blacklist entries
* Supported frameworks:
  * `gmock <https://google.github.io/googletest/>`_
  * `fff <https://github.com/meekrosoft/fff#fake-function-framework--fff>`_

Installation
============

Supported Platforms
-------------------

* `Arch Linux <https://archlinux.org/>`_
* `Fedora <https://getfedora.org/>`_
* `Ubuntu <https://ubuntu.com/>`_

Required Programs
-----------------

Depending on which compiler you prefer, chose either
* `gcc <https://gcc.gnu.org/>`_ 12.1.0 or
* `clang <https://clang.llvm.org/>`_ 14.0.0.

Also depending on what you prefer, chose either
* `make <https://www.gnu.org/software/make/>`_ or
* `ninja <https://ninja-build.org/>`_ 1.10.

In any case, you'll also need to following programs to build **ccmock** from
source:

* `bash <https://www.gnu.org/software/bash/bash.html>`_
* `cmake <https://cmake.org/>`_ 3.24
* `git <https://git-scm.com/>`_

Dependencies
------------

* `llvm <https://llvm.org/>`_ >= 14.0.0
* `clang <https://clang.llvm.org/>`_ >= 14.0.0


Procedure
---------

Install all required tools and libraries (chosing the most conservative
options):

.. code:: sh

   pacman -Syu llvm clang bash gcc git cmake make

.. code:: sh

   apt-get install llvm llvm-dev clang libclang-dev bash gcc g++ git cmake make 


Configure the build system:

.. code:: sh

   cmake -B build -DCMAKE_BUILD_TYPE=release


Build the project with clang:

.. code:: sh

   cmake --build build 

Install the built program to */usr/local/bin*:

.. code:: sh

   cmake --install build --strip


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
   

Commands
--------

Help Message
^^^^^^^^^^^^

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

.. code:: sh

   CCMOCK_CONFIG=<config>.yaml ccmock --config=<config>.yaml,<config>.yaml <input-file>

Dump Effective Configuration Settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock --dump-config

.. code:: sh

   ccmock --dump-config -o <output-file>

.. code:: sh

   ccmock --config=<config>.yaml,<config>.yaml --blacklist=<name> --dump-config

Pass Additional Compiler Flags to the Clang Frontend
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: sh

   ccmock --extra-args=-DNDEBUG,-Wall,-Werror <input-file>

Best Practices
--------------

* Always name your parameters in function declarations.
* Don't mock very fundamental functions like malloc(), open(), etc...
  as doing so will very likey break your unit test library in unpredictable
  ways.

Examples
========

Explore the `test/system <test/system>`_ directory to see how ccmock can be
integrated into a build system for automatic mock generation.

Open Points
===========

* Mocking C++ is not implemented
* Fake Function Framework generator not implemented
* Generate global variable definitions
* Restructure test tree
* Automatic include handling
* provide multiple test runners
* add support for another mock library (cpputest?)
* --backend None to just print prototypes
