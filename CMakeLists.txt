cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

# Allow for custom CMake modules
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

project(Ryme)

include(ProjectVersion)

###
### Options
###

option(
    RYME_TEST
    "Build Ryme test suite"
    OFF
)

option(
    RYME_BENCHMARK
    "Build Ryme with benchmarking"
    OFF
)

if(NOT CMAKE_BUILD_TYPE)

    list(JOIN "${CMAKE_CONFIGURATION_TYPES}" ", " _config_types)

    set(CMAKE_BUILD_TYPE "Release"
        CACHE STRING
        "Type of build to configure, can be one of: ${_config_types} [default: Release]"
        FORCE
    )

endif()

if(RYME_TEST)
    enable_testing()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###
### Third Party Dependencies
###

add_subdirectory(ThirdParty)

find_package(glm CONFIG REQUIRED)

find_package(fmt 8.0.0 CONFIG REQUIRED)

find_package(SDL2 2.0.6 CONFIG REQUIRED)

find_package(Python3 COMPONENTS Interpreter Development.Embed REQUIRED)

find_package(pybind11 2.8.0 CONFIG REQUIRED)

find_package(nlohmann_json CONFIG REQUIRED)

find_package(Vulkan COMPONENTS glslc REQUIRED)

find_package(spirv_cross_core CONFIG REQUIRED)

set(GTEST_MSVC_SEARCH "MD")
find_package(GTest REQUIRED)

###
### Globals
###

include(GetRuntimeDir)

# Find the location of SDL2.dll
ryme_get_runtime_dir(SDL2::SDL2 _SDL2_runtime_dir)

list(APPEND RYME_RUNTIME_PATH
    ${Python3_RUNTIME_LIBRARY_DIRS}
    ${_SDL2_runtime_dir}
)

list(APPEND RYME_ASSET_PATH
)

###
### Engine Library
###

add_subdirectory(Engine)

###
### Demo Executables
###

add_subdirectory(Demos)
