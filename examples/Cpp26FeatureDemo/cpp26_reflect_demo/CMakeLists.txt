cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(26)

project(cpp26_reflect_demo CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp26_reflect_demo ../cpp26_reflect_demo.cpp)
target_compile_options(cpp26_reflect_demo PRIVATE -freflection)
find_package(SQLite3 REQUIRED)
target_link_libraries(cpp26_reflect_demo SQLite::SQLite3)
