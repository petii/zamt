cmake_minimum_required(VERSION 3.12)

project(guishaders)

set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/share/${APPLICATION_NAME}/gui_shaders)
message(STATUS "GUI SHADER OUTPUT DIR: ${SHADER_OUTPUT_DIR}")

set(GLSL_SHADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/glsl_shader.vert
  ${CMAKE_CURRENT_SOURCE_DIR}/glsl_shader.frag
)

set(SPIRV_SHADERS
  ${SHADER_OUTPUT_DIR}/vert.spv
  ${SHADER_OUTPUT_DIR}/frag.spv
)

add_custom_command(OUTPUT ${SPIRV_SHADERS}
  COMMAND $ENV{VULKAN_SDK}/bin/glslangValidator -t -V ${GLSL_SHADERS}
  # BYPRODUCTS ${SPIRV_SHADERS}
  WORKING_DIRECTORY ${SHADER_OUTPUT_DIR}
  COMMENT "Compiling GLSL gui shaders"
  DEPENDS ${GLSL_SHADERS}
)

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

add_custom_target(gui_shaders ALL
  DEPENDS ${SPIRV_SHADERS}
)

install(
  FILES ${SPIRV_SHADERS}
  DESTINATION share/${APPLICATION_NAME}/gui_shaders
)