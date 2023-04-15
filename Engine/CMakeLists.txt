
set(_target RymeEngine)

###
### Source Files
###

file(
    GLOB_RECURSE
    _source_list
    "Public/*.h"
    "Public/*.hpp"
    "Private/*.h"
    "Private/*.hpp"
    "Private/*.c"
    "Private/*.cpp"
)

file(
    GLOB_RECURSE
    _source_input_list
    "Public/*.in"
    "Private/*.in"
)

include(ConfigureFileList)

ryme_configure_file_list("${_source_input_list}" _source_output_list)

###
### Asset Processing
###

file(GLOB_RECURSE
    _asset_list
    "Assets/*"
)

# Stop VS from trying to "build" our .obj model files
set_source_files_properties(
    ${_asset_list} 
    PROPERTIES
        HEADER_FILE_ONLY TRUE
)

list(PREPEND RYME_ASSET_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets
    ${CMAKE_CURRENT_BINARY_DIR}/Assets
)

###
### Shader Processing
###

file(
    GLOB_RECURSE
    _shader_input_list
    "Assets/*.glsl"
)

include(CompileShaderList)

ryme_compile_shader_list("${RYME_ASSET_PATH}" "${_shader_input_list}" _shader_output_list)

###
### Target Configuration
###

add_library(
    ${_target} SHARED
    ${_source_list}
    ${_source_output_list}
    ${_asset_list}
    ${_shader_output_list}
)

target_link_libraries(
    ${_target}
    PUBLIC
        glm::glm
        fmt::fmt
        SDL2::SDL2
        Python3::Python
        pybind11::embed
        nlohmann_json::nlohmann_json
        Vulkan::Vulkan
        spirv-cross-core
)

target_include_directories(
    ${_target}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
        ${CMAKE_CURRENT_BINARY_DIR}/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_BINARY_DIR}/Private
)

target_compile_definitions(
    ${_target}
    PUBLIC
        # Disable VS "not secure" warnings
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>

        $<$<BOOL:${RYME_BENCHMARK}>:RYME_ENABLE_BENCHMARK>

        # Configure vulkan.hpp to use vk::DispatchLoaderDynamic
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

set_target_properties(
    ${_target}
    PROPERTIES
        DEFINE_SYMBOL "RYME_EXPORT"
)

target_compile_options(
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

list(APPEND RYME_RUNTIME_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(RYME_RUNTIME_PATH ${RYME_RUNTIME_PATH} PARENT_SCOPE)

list(APPEND RYME_ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Assets)
set(RYME_ASSET_PATH ${RYME_ASSET_PATH} PARENT_SCOPE)
