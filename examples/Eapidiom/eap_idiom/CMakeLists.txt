cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(eap_idiom CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(eap_idiom ../eap_idiom.cpp)
