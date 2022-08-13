=========================
ccmock (Work in Progress)
=========================

.. image:: https://github.com/stnuessl/ccmock/actions/workflows/main.yaml/badge.svg
   :alt: CI
   :target: https://github.com/stnuessl/ccmock/actions

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
* Generate global variable definitions (how to configure this?)
* Automatic include handling
* provide multiple test runners
* add support for another mock library (cpputest?)
