
MACRO(DEFINE_DEMO _target)

    ###
    ### Source Files
    ###

    FILE(
        GLOB_RECURSE
        _source
        "Private/*.h"
        "Private/*.hpp"
        "Private/*.c"
        "Private/*.cpp"
    )
    
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

    ADD_EXECUTABLE(
        ${_target}
        ${_source}
        ${_asset_list}
        ${_shader_output_list}
    )

    TARGET_LINK_LIBRARIES(
        ${_target}
        PRIVATE
            RymeEngine
            SDL2::SDL2main
    )

    TARGET_INCLUDE_DIRECTORIES(
        ${_target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Private
    )

    TARGET_COMPILE_DEFINITIONS(
        ${_target}
        PRIVATE
            DEMO_NAME="${_target}"
    )

    SET_TARGET_PROPERTIES(
        ${_target}
        PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    IF(WIN32)
        SET(_ext ".exe")
    ENDIF()

    FILE(RELATIVE_PATH _executable ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${_target}${_ext})

    LIST(PREPEND RYME_ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Assets)

    EXECUTE_PROCESS(
        COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_SOURCE_DIR}/Scripts/generate-launch-targets.py
            --name "${_target}"
            --executable "${_executable}"
            --binary-dir "${CMAKE_BINARY_DIR}"
            --working-dir "${CMAKE_CURRENT_SOURCE_DIR}"
            --asset-path "${RYME_ASSET_PATH}"
            --runtime-path "${RYME_RUNTIME_PATH}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

ENDMACRO()

LIST(PREPEND RYME_ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Assets)

FILE(GLOB _demo_list ${CMAKE_CURRENT_SOURCE_DIR}/*)

FOREACH(_demo ${_demo_list})
    IF(EXISTS "${_demo}/CMakeLists.txt")
        ADD_SUBDIRECTORY(${_demo})
    ENDIF()
ENDFOREACH()
