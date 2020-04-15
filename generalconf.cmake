# global configuration

set(USE_ADDRESS_SANITIZER ON CACHE BOOL "Use -fsanitize=address for leak checking.")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Used toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()
