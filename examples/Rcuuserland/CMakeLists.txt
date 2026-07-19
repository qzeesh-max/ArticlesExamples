cmake_minimum_required(VERSION 3.10)
include("FindSmartGCC.cmake")
find_smart_gcc(26)
project(RcuuserlandBundle NONE)

add_subdirectory(rcu_userland_ex1)
