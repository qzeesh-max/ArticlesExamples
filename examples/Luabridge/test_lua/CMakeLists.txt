cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(test_lua CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_lua ../test_lua.cpp)
find_package(Lua REQUIRED)
find_package(Boost REQUIRED)
target_link_libraries(test_lua ${LUA_LIBRARIES})
target_include_directories(test_lua PRIVATE ${LUA_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
