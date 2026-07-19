cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(PointertomemberswithvariadicBundle NONE)

add_subdirectory(variadic_ptm_ex1)
add_subdirectory(variadic_ptm_ex2)
