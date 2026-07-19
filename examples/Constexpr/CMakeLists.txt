cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(ConstexprBundle NONE)

add_subdirectory(near_perfect_hash)
