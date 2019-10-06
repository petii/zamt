[![Build Status](https://travis-ci.com/gerazo/zamt.svg?branch=master)](https://travis-ci.com/gerazo/zamt)

# ZAMT

The "Z" Automatic Music Transcription Library and Testbed

## Build

cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER_ID=Clang ..

### build.sh

## Contributing

### Adding new module

#### Directory structure

    zamt dir
    ├── <module name>
    |   ├── include
    |   │   └── <header files>
    |   ├── src
    |   │   └── <source files>
    |   ├── test
    |   │   └── <source files for tests>
    |   ├── sources.cmake
    |   └── tests.cmake
    ├── modules.cmake
    └── targets.cmake

#### Files

sources.cmake
tests.cmake

modules.cmake
targets.cmake

#### ModuleCenter
