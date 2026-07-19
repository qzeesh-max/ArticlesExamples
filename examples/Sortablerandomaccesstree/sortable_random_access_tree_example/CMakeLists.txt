cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(sortable_random_access_tree_example CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sortable_random_access_tree_example ../sortable-random-access-tree-example.cpp)
find_package(Boost REQUIRED)
target_include_directories(sortable_random_access_tree_example PRIVATE ${Boost_INCLUDE_DIRS})
