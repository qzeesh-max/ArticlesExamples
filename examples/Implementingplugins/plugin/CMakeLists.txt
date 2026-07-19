cmake_minimum_required(VERSION 3.10)
include("../FindSmartGCC.cmake")
find_smart_gcc(14)

project(plugin CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Build the host executable
add_executable(plugin ../plugin.cpp ../PluginManager.cpp ../PluginInfo.cpp)

# Export symbols so plugins can resolve host functions (like PluginManager::Instance)
# - Windows: generates an import library (plugin.lib)
# - Linux: adds -rdynamic to export symbols
set_target_properties(plugin PROPERTIES ENABLE_EXPORTS TRUE)

# Trigger __declspec(dllexport) for the PluginManager when building the host
target_compile_definitions(plugin PRIVATE PLUGINDLL_EXPORT)

if(NOT WIN32)
    target_link_libraries(plugin dl)
endif()

# 2. Build the plugin shared library
add_library(TestPluginOne SHARED ../TestPluginOne.cpp)

# Ensure the library output exactly matches what plugin.cpp expects: "libTestPluginOne.so" or "libTestPluginOne.dll"
set_target_properties(TestPluginOne PROPERTIES PREFIX "lib")

if(WIN32)
    # On Windows, plugins MUST link against the host executable's import library to resolve symbols
    target_link_libraries(TestPluginOne PRIVATE plugin)
elseif(APPLE)
    # On macOS, allow undefined symbols to be resolved at runtime by the host
    set_target_properties(TestPluginOne PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
    set_target_properties(TestPluginOne PROPERTIES SUFFIX ".so")
else()
    # On Linux, CMake naturally outputs libTestPluginOne.so
    # ELF natively supports unresolved symbols in shared libs, and the host's -rdynamic ensures it resolves at runtime
endif()
