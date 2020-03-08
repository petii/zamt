# global configuration

set(USE_ADDRESS_SANITIZER ON CACHE BOOL "Use -fsanitize=address for leak checking.")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_AR gcc-ar)
  set(CMAKE_RANLIB gcc-ranlib)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_C_COMPILER clang)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_AR llvm-ar)
  set(CMAKE_RANLIB llvm-ranlib)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld)
endif()
