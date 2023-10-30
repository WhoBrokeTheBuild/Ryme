
macro(ryme_define_demo _target)

    ###
    ### Source Files
    ###

    file(
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

    add_executable(
        ${_target}
        ${_source}
        ${_asset_list}
        ${_shader_output_list}
    )

    target_link_libraries(
        ${_target}
        PRIVATE
            RymeEngine
            SDL2::SDL2main
    )

    target_include_directories(
        ${_target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Private
    )

    target_compile_definitions(
        ${_target}
        PRIVATE
            DEMO_NAME="${_target}"
    )

    if(WIN32)
        set(_ext ".exe")
    endif()

    file(RELATIVE_PATH _executable ${CMAKE_BINARY_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_target}${_ext})

    list(PREPEND RYME_ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Assets)

    execute_process(
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

endmacro()

list(PREPEND RYME_ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Assets)

file(GLOB _demo_list ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(_demo ${_demo_list})
    if(EXISTS "${_demo}/CMakeLists.txt")
        add_subdirectory(${_demo})
    endif()
endforeach()
