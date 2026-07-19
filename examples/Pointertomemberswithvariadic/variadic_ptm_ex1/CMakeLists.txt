cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(variadic_ptm_ex1 CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(variadic_ptm_ex1 ../variadic-ptm-ex1.cpp)
