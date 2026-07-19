cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(ImplementingpluginsBundle NONE)

add_subdirectory(plugin)
