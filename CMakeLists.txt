cmake_minimum_required(VERSION 3.31)
project(zirc CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_STANDARD C++14)

add_subdirectory(zirc)
add_subdirectory(platform)
