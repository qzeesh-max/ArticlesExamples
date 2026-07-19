cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(Cpp26AnnotationsBundle NONE)

add_subdirectory(annotations_example)
