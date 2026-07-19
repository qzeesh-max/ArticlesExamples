cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(robust_shareable_primitives CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(robust_shareable_primitives ../robust_shareable_primitives.cpp)
find_package(Boost REQUIRED)
target_include_directories(robust_shareable_primitives PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(robust_shareable_primitives pthread)
if(UNIX AND NOT APPLE)
    target_link_libraries(robust_shareable_primitives rt)
endif()
