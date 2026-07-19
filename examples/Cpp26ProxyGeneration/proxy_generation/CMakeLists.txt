cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(26)

project(proxy_generation CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(proxy_generation ../proxy_generation.cpp)
target_compile_options(proxy_generation PRIVATE -freflection)
