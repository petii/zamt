cmake_minimum_required(VERSION 3.12)

project(guihandler)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gui_shaders)

set(IMGUI_ROOT ${PROJECT_ROOT}/libs/imgui)
find_package(Imgui)
find_package(Vulkan)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include/${PROJECT_NAME})

set(SOURCES 
  ${SOURCE_DIR}/guihandler.cpp
  ${SOURCE_DIR}/vulkanobjects.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/guihandler.h
)

set(IMPL_HEADERS

  ${INCLUDE_DIR}/vulkanobjects.h
)

add_library(guihandler
  ${HEADERS}
  ${IMPL_HEADERS}
  ${SOURCES}
)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC EXECUTABLE_NAME="${APPLICATION_NAME}"
)

set_target_properties(guihandler PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${INCLUDE_DIR}
)

target_include_directories(guihandler
  PRIVATE ${INCLUDE_DIR}
          $<TARGET_PROPERTY:dearimgui,INTERFACE_INCLUDE_DIRECTORIES>
          $<TARGET_PROPERTY:windowhandler,INTERFACE_INCLUDE_DIRECTORIES>
          $<TARGET_PROPERTY:graphicshandler,INCLUDE_DIRECTORIES>
)

target_link_libraries(guihandler
  PRIVATE Vulkan::Vulkan
          dearimgui
)

install(
  TARGETS guihandler
  DESTINATION lib
)
install(
  FILES ${HEADERS}
  DESTINATION include
)
install(
  FILES ${IMPL_HEADERS}
  DESTINATION include/detail
)
