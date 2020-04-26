# The "Z" Automatic Music Transcription Library and Testbed
[![Build Status](https://travis-ci.com/gerazo/zamt.svg?branch=master)](https://travis-ci.com/gerazo/zamt)

## Build

If you have all needed dependencies, the build can be as simple as

```console
$ mkdir build
$ cd build
$ cmake ..
$ make
```

To install dependencies, on debian-based systems you can use the the [build script](https://github.com/gerazo/zamt/blob/master/build.sh).

### build.sh



### CMake Parameters

* ``CMAKE_BUILD_TYPE``: Commonly ``Debug`` or ``Release``, but there are other variations as well
* ``CMAKE_TOOLCHAIN_FILE``: Which compiler toolchain to use. Available ones are here: https://github.com/petii/zamt/tree/master/cmake/toolchain
* ``USE_ADDRESS_SANITIZER``: Self explanatory. Only affects ``Debug`` builds.
* ``USE_UB_SANITIZER``: Build with undefined behaviour sanitizer. (Unused)

## Project structure
