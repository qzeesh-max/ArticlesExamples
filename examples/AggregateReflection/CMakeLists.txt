cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(AggregateReflectionBundle NONE)

add_subdirectory(test_struct_reflector)
