cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(20)

project(simple_http_server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(simple_http_server ../simple_http_server.cpp ../io_uring_coroutine_helpers.cpp)
find_package(Boost REQUIRED)
target_include_directories(simple_http_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(simple_http_server uring)
