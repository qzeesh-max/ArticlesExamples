cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(AugmentedredblacktreeBundle NONE)

add_subdirectory(augmented_rbt_ex1)
