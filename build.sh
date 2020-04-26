#!/bin/sh

TOOLCHAIN_FILES=$(ls -d cmake/toolchain/*)
MODES="Debug Release"
#MODES="Debug Release RelWithDebInfo"
CMAKE_PROJECT="Ninja"
#CMAKE_PROJECT="Eclipse CDT4 - Ninja"

BUILD_DEPS="cmake ninja-build binutils g++ llvm-dev clang clang-format"
PULSEAUDIO_DEPS="libpulse-dev"
GTKMM_DEPS="libgtkmm-3.0-dev"
FFTW_DEPS="libfftw3-dev"

ALL_DEPS="$BUILD_DEPS $PULSEAUDIO_DEPS $GTKMM_DEPS $FFTW_DEPS"
ONLYDEP=0
KEEPGOING=1
USEASAN=ON



while [ $# -gt 0 ]; do
  case "$1" in
    -compilers) shift; TOOLCHAINS="$1" ;;
    -list-compilers) ls cmake/toolchain/* | xargs -I file basename file Toolchain.cmake
                     exit 98
                     ;;
    -k) shift; KEEPGOING="$1" ;;
    -check-dependencies) ONLYDEP=1 ;;
    -nodep) NODEP=1 ;;
    -noanal) NOANAL=1 ;;
    -noasan) USEASAN=OFF ;;
    -noubsan) USEUBSAN=OFF ;;
    -v) VERBOSE="-v" ;;
    *) echo "ZAMT build script parameters:"
       echo "  -compilers <compiler list>   Tests with the given compiler toolchains."
       echo "  -list-compilers              List the available compiler toolchains"
       echo "  -k <N>                       Wait for N errors before stopping."
       echo "  -check-dependencies          Exit after dependency detection and installation"
       echo "  -nodep                       Skip dependency detection and installation."
       echo "  -noanal                      Skip extra analysis of sources."
       echo "  -noasan                      Do not use leak checking in containers."
       echo "  -noubsan                     Do not use undefined behaviour sanitizer."
       echo "  -v                           Give verbose output on everything."
       exit 99
       ;;
  esac
  shift
done

if [ "$NODEP" != "1" ]; then
  if command -v apt >/dev/null && command -v dpkg >/dev/null ; then
    if ! dpkg -s $ALL_DEPS >/dev/null; then
      echo "\\033[1m\\033[37m\\033[42m--- Installing Dependencies (needs sudo privileges)...\\033[0m"
      sudo apt install $ALL_DEPS
    fi
  else
    # TODO write commands for other packaging systems
    echo "\\033[1m\\033[37m\\033[41m" This packaging system is not yet supported!!! "\\033[0m" These dependencies should be installed: $ALL_DEPS
  fi
  if [ $ONLYDEP ]; then
    exit 97
  fi
fi

for TOOLCHAIN_FILE in $TOOLCHAIN_FILES; do
  TOOLCHAIN=$(basename $TOOLCHAIN_FILE "Toolchain.cmake")
  if [ -n TOOLCHAINS ]; then
    if ! echo $TOOLCHAINS | grep $TOOLCHAIN - ; then
      continue
    fi
  fi
  TOOLCHAIN_FILE=./$TOOLCHAIN_FILE
  for MODE in $MODES; do
    BUILD_DIR="_build_"$TOOLCHAIN"_"$MODE

    if [ ! -d $BUILD_DIR ]; then
      echo "\\033[1m\\033[37m\\033[42m" Creating new build setup: $BUILD_DIR "\\033[0m"
      mkdir -p $BUILD_DIR
      cd $BUILD_DIR
      cmake -G "$CMAKE_PROJECT" -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE -DCMAKE_BUILD_TYPE=$MODE -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_ADDRESS_SANITIZER=$USEASAN -DUSE_UB_SANITIZER=$USEUBSAN ..
      cd ..
    fi

    echo "\\033[1m\\033[37m\\033[42m" Compiling: $BUILD_DIR "\\033[0m"
    cd $BUILD_DIR
    if ninja -k $KEEPGOING $VERBOSE ; then
      echo "\\033[1m\\033[37m\\033[42m" Testing: $BUILD_DIR "\\033[0m"
      if CTEST_OUTPUT_ON_FAILURE=TRUE ninja test $VERBOSE ; then
        if [ "$VERBOSE" != "" ]; then
          echo "\\033[1m\\033[37m\\033[42m" Test log of $BUILD_DIR "\\033[0m"
          cat Testing/Temporary/LastTest.log
        fi
        echo "\\033[1m\\033[37m\\033[42m" Finished $BUILD_DIR "\\033[0m"
      else
        if [ "$VERBOSE" != "" ]; then
          echo "\\033[1m\\033[37m\\033[41m" Test log of $BUILD_DIR "\\033[0m"
          cat Testing/Temporary/LastTest.log
        fi
        echo "\\033[1m\\033[37m\\033[41m" Testing of $BUILD_DIR failed."\\033[0m"
        cd ..
        exit 2
      fi
    else
      echo "\\033[1m\\033[37m\\033[41m" Compilation of $BUILD_DIR failed."\\033[0m"
      cd ..
      exit 1
    fi
    cd ..

  done
done

if [ "$NOANAL" != "1" ]; then
  for MODULE in $( ls -1d */ | grep -v _build_ ); do
    echo "\\033[1m\\033[37m\\033[42m" Analyzing module $MODULE "\\033[0m"
    for SOURCE in $( find $MODULE"include" $MODULE"src" $MODULE"test" -regex "\(.*\.cpp\)\|\(.*\.h\)" ); do
      if clang-format -style=file -output-replacements-xml $SOURCE | grep "<replacement " >/dev/null; then
        echo "\\033[1m\\033[37m\\033[43m" Syntax convention problem with $SOURCE "\\033[0m"
        clang-format -style=file $SOURCE | diff -u $SOURCE -
      fi
    done
  done
fi

echo "\\033[1m\\033[37m\\033[42m Build successful. \\033[0m"
exit 0

