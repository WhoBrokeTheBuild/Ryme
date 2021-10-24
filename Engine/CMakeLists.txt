
SET(_target RymeEngine)

###
### Source Files
###

FILE(
    GLOB_RECURSE
    _source
    "Public/*.h"
    "Public/*.hpp"
    "Private/*.c"
    "Private/*.cpp"
    "Private/*.c"
    "Private/*.cpp"
)

FILE(
    GLOB_RECURSE
    _source_in
    "Public/*.in"
    "Private/*.in"
)

INCLUDE(ConfigureFileList)

CONFIGURE_FILE_LIST("${_source_in}" _source_out)

###
### Target Configuration
###

ADD_LIBRARY(
    ${_target} SHARED
    ${_source}
    ${_source_out}
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
        # Configure VS to use C++20, since it ignores CXX_STANDARD
        $<$<CXX_COMPILER_ID:MSVC>: /std:c++latest>
		
        # Configure exception handling model
        $<$<CXX_COMPILER_ID:MSVC>: /EHs>

        # Force windows to use UTF-8
        $<$<CXX_COMPILER_ID:MSVC>: /utf-8>

        # Enable most warnings, disable unknown pragmas warning
        $<$<CXX_COMPILER_ID:GNU>:   -Wall -Wno-unknown-pragmas>
        $<$<CXX_COMPILER_ID:Clang>: -Wall -Wno-unknown-pragmas -Wno-nullability-completeness>
        $<$<CXX_COMPILER_ID:MSVC>:  /wd4068>
)

LIST(APPEND RUNTIME_PATH ${CMAKE_CURRENT_BINARY_DIR})
SET(RUNTIME_PATH ${RUNTIME_PATH} PARENT_SCOPE)