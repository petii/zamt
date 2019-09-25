cmake_minimum_required(VERSION 3.12)

project(graphicshandler)

find_package(Vulkan REQUIRED)
find_package(glm)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${PROJECT_ROOT}/include/${PROJECT_NAME})

set(SOURCES 
  ${SOURCE_DIR}/graphicshandler.cpp
  ${SOURCE_DIR}/vulkanimpl.cpp
  
  ${SOURCE_DIR}/debugutils.cpp
  ${SOURCE_DIR}/swapchains.cpp
  ${SOURCE_DIR}/shaders.cpp
  ${SOURCE_DIR}/pipelines.cpp
  ${SOURCE_DIR}/commands.cpp
  ${SOURCE_DIR}/rendering.cpp
  ${SOURCE_DIR}/buffers.cpp
  ${SOURCE_DIR}/images.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/graphicshandler.h  
  
  ${PROJECT_ROOT}/include/common.h
)
set(IMPL_HEADERS
  ${INCLUDE_DIR}/detail/vulkanimpl.h
  ${INCLUDE_DIR}/detail/debugutils.h
  ${INCLUDE_DIR}/detail/swapchains.h
  ${INCLUDE_DIR}/detail/shaders.h
  ${INCLUDE_DIR}/detail/pipelines.h
  ${INCLUDE_DIR}/detail/rendering.h
  ${INCLUDE_DIR}/detail/buffers.h
  ${INCLUDE_DIR}/detail/images.h
)

add_library(graphicshandler
  ${HEADERS}
  ${IMPL_HEADERS}
  ${SOURCES}
)

set_target_properties(graphicshandler PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${INCLUDE_DIR}
)

target_include_directories(graphicshandler
PRIVATE ${INCLUDE_DIR}
        ${INCLUDE_DIR}/detail
        $<TARGET_PROPERTY:windowhandler,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:guihandler,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:glm,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Vulkan::Vulkan,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:${APPLICATION_NAME},INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(graphicshandler
  PRIVATE Vulkan::Vulkan
          windowhandler
          guihandler
          Vulkan::Vulkan
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/guihandler)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaders)

install(
  TARGETS graphicshandler
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