cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(20)

project(reflection_example CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(reflection_example ../reflection_example.cpp)
target_include_directories(reflection_example PRIVATE ../)
find_package(Boost REQUIRED)
target_link_libraries(reflection_example Boost::headers)
