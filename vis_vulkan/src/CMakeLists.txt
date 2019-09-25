cmake_minimum_required(VERSION 3.12)

project(${APPLICATION_NAME})

find_package(Boost COMPONENTS program_options filesystem)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_ROOT}/include)

set(SOURCES 
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/visualizationapp.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/visualizationapp.h
  ${INCLUDE_DIR}/common.h
  ${INCLUDE_DIR}/window_function.h
  ${INCLUDE_DIR}/shader_directories.h
)

add_executable(${PROJECT_NAME}
  ${HEADERS}
  ${SOURCES}
)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC EXECUTABLE_NAME="${APPLICATION_NAME}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${INCLUDE_DIR}
)

target_include_directories(${PROJECT_NAME} 
  PRIVATE windowhandler
          audiohandler
          fourierhandler
          graphicshandler
          ${INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Boost::program_options
          Boost::filesystem
          windowhandler
          audiohandler
          fourierhandler
          graphicshandler
          stdc++fs
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/windowhandler)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/audiohandler)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fourierhandler)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/graphicshandler)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION bin
)
install(
  FILES ${HEADERS}
  DESTINATION include
)