cmake_minimum_required(VERSION 2.6)

project(libpm)

include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
