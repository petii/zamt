cmake_minimum_required(VERSION 3.12)

project(windowhandler)

find_package(glfw3 REQUIRED)
find_package(Vulkan)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include/${PROJECT_NAME})

set(SOURCES 
  ${SOURCE_DIR}/windowhandler.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/windowhandler.h  
)

add_library(windowhandler
  ${HEADERS}
  ${SOURCES}
)

set_target_properties(windowhandler PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${INCLUDE_DIR}
)

target_include_directories(windowhandler
  PRIVATE ${INCLUDE_DIR}
          $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
          $<TARGET_PROPERTY:Vulkan::Vulkan,INTERFACE_INCLUDE_DIRECTORIES>          
)

target_link_libraries(windowhandler
  PRIVATE glfw
          Vulkan::Vulkan
)

install(
  TARGETS windowhandler
  DESTINATION lib
)
install(
  FILES ${HEADERS}
  DESTINATION include
)