[![CI](https://github.com/stnuessl/ccmock/actions/workflows/main.yaml/badge.svg)](https://github.com/stnuessl/ccmock/actions/workflows/main.yaml)

Information within this README might already be outdated.

Current Status of the project
* With luck you'll be able to generate C function mocks
* Doing the implementation for  C++ function mocking support has not even 
started yet.

# ccmock (Work in Progress)

A command-line tool based on clang and llvm for the automatic generation of 
function mocks for C and C++ projects using Google's 
[gtest and gmock](https://google.github.io/googletest/).

## About

### Motivation

### Advantages

* Easy mock generation which can be automated via a build system.

### Disadvantages

* _Slow_ as C++ is getting parsed by clang's compiler frontend

## Features (Goals)

* Easy/Conventent to use for automation within a build system
* Easy/Convenient to use on the command-line

## Usage

### Configuration Precedence Order

1. Command-line arguments
1. Environment variables
1. File-specific configuration file
1. Project-specific configuration file


### Examples

Write generated mocks for __source.c__ to __source-mocks.hpp__.

```
$ ccmock -o source-mocks.inc source.c
```

Write generated mocks to stdout.
```
$ ccmock source.cpp 
```

Usually _ccmock_ will need additional information to be able to parse the
input file. Use _ccmock_ with a compilation database:
```
$ ccmock --compile-commands=compile_commands.json source.cpp
```

Use _ccmock_ with a configuration file:
```
$ ccmock -c/--config ccmock.yaml source.cpp
```

### Integration into make

```make

source.elf: source.o test-source.o
    <linker-invocation>

source.o: source.c
    <compiler-invocation>

test-source.o: test-source.cpp test-source.inc
    <compiler-invocation>

test-source.inc: source.c
    ccmock -o $@ $<

```
