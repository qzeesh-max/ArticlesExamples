cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(PointertomembertemplateargumentsBundle NONE)

add_subdirectory(ptm_ex1)
add_subdirectory(ptm_ex2)
