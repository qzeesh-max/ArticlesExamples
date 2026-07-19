cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(vartempl_ex4 CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(vartempl_ex4 ../vartempl-ex4.cpp)
