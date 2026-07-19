cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(rcu_userland_ex1 CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rcu_userland_ex1 ../rcu-userland-ex1.cpp)
target_link_libraries(rcu_userland_ex1 pthread)
