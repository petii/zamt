cmake_minimum_required(VERSION 3.12)

file(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

set(SHADER_DIRECTORIES "")
foreach(file ${files})
set(fullpath ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  # message(STATUS ${fullpath})
  if(IS_DIRECTORY ${fullpath})
    # message(STATUS "BINGO! ${fullpath}")
    list(APPEND SHADER_DIRECTORIES ${file})
  endif()
endforeach()

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIRECTORY})
message(STATUS "FOUND SHADERS: ${SHADER_DIRECTORIES}")

set(SHADER_SRCS "")
set(SHADER_FILES "")
set(GENERATORS "")
set(INDICES "")

foreach(shader ${SHADER_DIRECTORIES})
  file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIRECTORY}/${shader})
  set(GLSL_SHADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/shader.vert 
    ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/shader.frag
  )
  set(SPIRV_SHADERS 
    ${SHADER_OUTPUT_DIRECTORY}/${shader}/vert.spv 
    ${SHADER_OUTPUT_DIRECTORY}/${shader}/frag.spv
  )

  add_custom_command(OUTPUT ${SPIRV_SHADERS}
    COMMAND $ENV{VULKAN_SDK}/bin/glslangValidator -t -V ${GLSL_SHADERS}
    WORKING_DIRECTORY ${SHADER_OUTPUT_DIRECTORY}/${shader}
    COMMENT "Compiling GLSL shaders for ${shader}"
    DEPENDS ${GLSL_SHADERS}
  )
  
  list(APPEND SHADER_SRCS ${GLSL_SHADERS})
  list(APPEND SHADER_FILES ${SPIRV_SHADERS})
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/generate_indices.py)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/generate_indices.py 
      ${SHADER_OUTPUT_DIRECTORY}/${shader}/generate_indices.py
      COPYONLY
    )
    set(GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/generate_indices.py)
    list(APPEND GENERATORS ${GENERATOR})
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/indices.txt)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/indices.txt 
      ${SHADER_OUTPUT_DIRECTORY}/${shader}/indices.txt
      COPYONLY
    )
    set(INDICES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${shader}/indices.txt)
    list(APPEND INDICES ${INDICES_FILE})
  else()
    message(WARNING "Found shader ${shader} without indices.txt or generator script")
    endif()
  install(
    FILES ${GLSL_SHADERS} ${SPIRV_SHADERS} ${GENERATOR} ${INDICES_FILE}
    DESTINATION share/${APPLICATION_NAME}/shaders/${shader}
  )
endforeach()

add_custom_target(shaders ALL
  DEPENDS ${SHADER_FILES} ${GENERATORS} ${INDICES}
)
