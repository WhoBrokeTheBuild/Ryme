
SET(_target RymeEngine)

###
### Source Files
###

FILE(
    GLOB_RECURSE
    _source_list
    "Public/*.h"
    "Public/*.hpp"
    "Private/*.h"
    "Private/*.hpp"
    "Private/*.c"
    "Private/*.cpp"
)

FILE(
    GLOB_RECURSE
    _source_input_list
    "Public/*.in"
    "Private/*.in"
)

INCLUDE(ConfigureFileList)

CONFIGURE_FILE_LIST("${_source_input_list}" _source_output_list)

###
### Asset Processing
###

FILE(GLOB_RECURSE
    _asset_list
    "Assets/*"
)

# Stop VS from trying to "build" our .obj model files
SET_SOURCE_FILES_PROPERTIES(
    ${_asset_list} 
    PROPERTIES
        HEADER_FILE_ONLY TRUE
)

LIST(PREPEND RYME_ASSET_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets
    ${CMAKE_CURRENT_BINARY_DIR}/Assets
)

###
### Shader Processing
###

FILE(
    GLOB_RECURSE
    _shader_input_list
    "Assets/*.glsl"
)

INCLUDE(CompileShaderList)

COMPILE_SHADER_LIST("${RYME_ASSET_PATH}" "${_shader_input_list}" _shader_output_list)

###
### Target Configuration
###

ADD_LIBRARY(
    ${_target} SHARED
    ${_source_list}
    ${_source_output_list}
    ${_asset_list}
    ${_shader_output_list}
)

TARGET_LINK_LIBRARIES(
    ${_target}
    PUBLIC
        glm::glm
        fmt::fmt
        SDL2::SDL2
        Python3::Python
        pybind11::headers
        Vulkan::Vulkan
)

TARGET_INCLUDE_DIRECTORIES(
    ${_target}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
        ${CMAKE_CURRENT_BINARY_DIR}/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_BINARY_DIR}/Private
)

TARGET_COMPILE_DEFINITIONS(
    ${_target}
    PUBLIC
        # Disable VS "not secure" warnings
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>

        $<$<BOOL:${RYME_BENCHMARK}>:RYME_ENABLE_BENCHMARK>

        # Configure vulkan.hpp to use vk::DispatchLoaderDynamic
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

SET_TARGET_PROPERTIES(
    ${_target}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        DEFINE_SYMBOL "RYME_EXPORT"
)

TARGET_COMPILE_OPTIONS(
    ${_target}
    PUBLIC
        # Recommended by pybind11
        $<$<CXX_COMPILER_ID:MSVC>: /permissive->
		
        # Configure exception handling model
        $<$<CXX_COMPILER_ID:MSVC>: /EHs>

        # Force windows to use UTF-8
        $<$<CXX_COMPILER_ID:MSVC>: /utf-8>

        # Enable most warnings, disable unknown pragmas warning
        $<$<CXX_COMPILER_ID:GNU>:   -Wall -Wno-unknown-pragmas>
        $<$<CXX_COMPILER_ID:Clang>: -Wall -Wno-unknown-pragmas -Wno-nullability-completeness -Wno-self-assign-overloaded>
        $<$<CXX_COMPILER_ID:MSVC>:  /wd4068>
)

LIST(APPEND RYME_RUNTIME_PATH ${CMAKE_CURRENT_BINARY_DIR})
SET(RYME_RUNTIME_PATH ${RYME_RUNTIME_PATH} PARENT_SCOPE)

LIST(APPEND RYME_ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Assets)
SET(RYME_ASSET_PATH ${RYME_ASSET_PATH} PARENT_SCOPE)
