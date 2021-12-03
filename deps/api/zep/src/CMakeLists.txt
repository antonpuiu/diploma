cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE ZEP_SOURCES ${GFXF_ROOT_DIR}/deps/api/zep/*.c*)

add_library(zep SHARED ${ZEP_SOURCES})
set_target_properties(zep PROPERTIES PUBLIC_HEADER ${GFXF_ROOT_DIR}/deps/api/zep/zep.h)
