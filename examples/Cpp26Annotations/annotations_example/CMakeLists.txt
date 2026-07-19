cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(26)

project(annotations_example CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(annotations_example ../annotations_example.cpp)
target_compile_options(annotations_example PRIVATE -freflection)
