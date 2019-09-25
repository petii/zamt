cmake_minimum_required(VERSION 3.12)

project(fourierhandler)

find_package(FFTW3)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include/${PROJECT_NAME})

set(SOURCES 
  ${SOURCE_DIR}/fourierhandler.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/fourierhandler.h  
)

add_library(fourierhandler
  ${HEADERS}
  ${SOURCES}
)

set_target_properties(fourierhandler PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${INCLUDE_DIR}
)

target_include_directories(fourierhandler
  PRIVATE ${INCLUDE_DIR}
          $<TARGET_PROPERTY:fftw::fftw3f,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(fourierhandler
  PRIVATE fftw::fftw3f
)

install(
  TARGETS fourierhandler
  DESTINATION lib
)
install(
  FILES ${HEADERS}
  DESTINATION include
)