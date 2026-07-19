cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(ReflectionBundle NONE)

add_subdirectory(reflection_example)
