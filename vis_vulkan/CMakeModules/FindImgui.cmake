


find_path(IMGUI_PATH 
  NAMES imgui.h imgui.cpp
  HINTS ${IMGUI_ROOT}
)

message(STATUS "IMGUI PATH: ${IMGUI_PATH}")

set(IMGUI_SOURCES
  ${IMGUI_PATH}/imgui.cpp
  ${IMGUI_PATH}/imgui.cpp
  ${IMGUI_PATH}/imgui_demo.cpp
  ${IMGUI_PATH}/imgui_draw.cpp
  ${IMGUI_PATH}/imgui_widgets.cpp
) 

set(IMGUI_HEADERS
  ${IMGUI_PATH}/imgui.h
  ${IMGUI_PATH}/imgui_internal.h
  ${IMGUI_PATH}/imconfig.h
  ${IMGUI_PATH}/imstb_rectpack.h
  ${IMGUI_PATH}/imstb_textedit.h
  ${IMGUI_PATH}/imstb_truetype.h
)

add_library(dearimgui STATIC
  ${IMGUI_HEADERS}
  ${IMGUI_SOURCES}
)

set(IMGUI_INCLUDE_DIR ${IMGUI_PATH})

set_target_properties(dearimgui PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${IMGUI_INCLUDE_DIR}
)

target_include_directories(dearimgui
  INTERFACE ${IMGUI_INCLUDE_DIR}
)

target_link_libraries(dearimgui
  
)

set(IMGUI_SRCS ${IMGUI_HEADERS} ${IMGUI_SOURCES})