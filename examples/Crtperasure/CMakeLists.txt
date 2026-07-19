cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(CrtperasureBundle NONE)

add_subdirectory(template_variant)
