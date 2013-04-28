cmake_minimum_required (VERSION 2.6)
# TODO: put Project name here
set (PROJECT_NAME onyx-i62hd-sdl)
set (OUTPUT_NAME ${PROJECT_NAME})
project (${PROJECT_NAME})

# Additional cmake modules
set     (CMAKE_MODULE_PATH  "${CMAKE_SOURCE_DIR}/cmake_modules")

# pre-configure
include (mymacro)

# configure source code
add_subdirectory(src)
