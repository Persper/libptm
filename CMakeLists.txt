cmake_minimum_required(VERSION 2.6)

project(libpm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(pm naming.cc utils/MurmurHash3.cpp)
add_executable(cmain main.c)
add_executable(ccmain main.cc)

target_link_libraries(cmain pm)
target_link_libraries(ccmain pm)
