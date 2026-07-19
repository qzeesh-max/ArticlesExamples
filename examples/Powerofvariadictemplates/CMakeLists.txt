cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(PowerofvariadictemplatesBundle NONE)

add_subdirectory(vartempl_ex3)
add_subdirectory(vartempl_ex4)
add_subdirectory(vartempl_ex5)
