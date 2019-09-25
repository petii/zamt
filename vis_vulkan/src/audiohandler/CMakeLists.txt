cmake_minimum_required(VERSION 3.12)

project(audiohandler)

find_package(PortAudioCpp REQUIRED)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include/${PROJECT_NAME})

set(SOURCES 
  ${SOURCE_DIR}/audiohandler.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/audiohandler.h  
)

add_library(audiohandler
  ${HEADERS}
  ${SOURCES}
)

set_target_properties(audiohandler PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${INCLUDE_DIR}
)

target_include_directories(audiohandler
  PRIVATE ${INCLUDE_DIR}
          $<TARGET_PROPERTY:portaudio::portaudiocpp,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(audiohandler
  PRIVATE portaudio::portaudiocpp
)

install(
  TARGETS audiohandler
  DESTINATION lib
)
install(
  FILES ${HEADERS}
  DESTINATION include
)