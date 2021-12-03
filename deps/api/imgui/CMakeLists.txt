cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE IMGUI_SOURCES ${GFXF_ROOT_DIR}/deps/api/imgui/*.c*)

find_package(Freetype REQUIRED)
include_directories(/usr/include/freetype2)

add_library(imgui SHARED ${IMGUI_SOURCES})
set_target_properties(imgui PROPERTIES PUBLIC_HEADER ${GFXF_ROOT_DIR}/deps/api/imgui/imgui.h)
target_link_libraries(imgui dl freetype)
target_compile_definitions(imgui PRIVATE IMGUI_ENABLE_FREETYPE)
