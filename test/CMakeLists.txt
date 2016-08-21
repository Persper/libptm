cmake_minimum_required(VERSION 2.6.2)

add_subdirectory(gtest)

include_directories(gtest/include gtest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
cxx_executable(allocator_unittest . gtest_main)
add_test(allocator_unittest allocator_unittest)
