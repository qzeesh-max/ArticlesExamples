cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(Cpp26FeatureDemoBundle NONE)

add_subdirectory(cpp26_reflect_demo)
