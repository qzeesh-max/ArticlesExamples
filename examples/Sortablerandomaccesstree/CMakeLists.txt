cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(SortablerandomaccesstreeBundle NONE)

add_subdirectory(sortable_random_access_tree_example)
