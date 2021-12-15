cmake_minimum_required(VERSION 3.16)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ZEP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.c*)

find_package(Freetype REQUIRED)
include_directories(/usr/include/freetype2)

add_library(zep SHARED ${ZEP_SOURCES})
set_target_properties(zep PROPERTIES PUBLIC_HEADER ${GFXF_ROOT_DIR}/deps/api/imgui/imgui.h)
target_link_libraries(zep dl freetype imgui stdc++fs)

target_include_directories(zep PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/deps/api/imgui)


add_definitions(-DZEP_FEATURE_CPP_FILE_SYSTEM)
