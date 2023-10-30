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
    include(CTest)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

###
### Third Party Dependencies
###

find_package(Python3 3.10 COMPONENTS Interpreter Development.Embed REQUIRED)

find_package(Vulkan COMPONENTS glslc REQUIRED)

include(FetchContent)

FetchContent_Declare(
    fmt
    URL "https://github.com/fmtlib/fmt/archive/refs/tags/10.1.1.zip"
    FIND_PACKAGE_ARGS 10.1.1
)

FetchContent_Declare(
    glm
    URL "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip"
)

FetchContent_Declare(
    GTest
    URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"
    FIND_PACKAGE_ARGS 1.14.0
)

FetchContent_Declare(
    nlohmann_json
    URL "https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.zip"
    FIND_PACKAGE_ARGS 3.11.2
)

FetchContent_Declare(
    pybind11
    URL "https://github.com/pybind/pybind11/archive/refs/tags/v2.11.1.zip"
    FIND_PACKAGE_ARGS 2.11.1
)

FetchContent_Declare(
    SDL2
    URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.28.4.zip"
    FIND_PACKAGE_ARGS 2.28.4
)

FetchContent_Declare(
    spirv_cross
    URL "https://github.com/KhronosGroup/SPIRV-Cross/archive/refs/heads/main.zip"
)

FetchContent_MakeAvailable(glm fmt SDL2 pybind11 nlohmann_json spirv_cross GTest)

###
### Globals
###

# include(GetRuntimeDir)

# Find the location of SDL2.dll
# ryme_get_runtime_dir(SDL2::SDL2 _SDL2_runtime_dir)

list(APPEND RYME_RUNTIME_PATH
    # ${Python3_RUNTIME_LIBRARY_DIRS}
    # ${_SDL2_runtime_dir}
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
