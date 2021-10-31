
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
    ### Target Configuration
    ###

    ADD_EXECUTABLE(
        ${_target}
        ${_source}
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

FILE(GLOB _demo_list ${CMAKE_CURRENT_SOURCE_DIR}/*)

FOREACH(_demo ${_demo_list})
    IF(IS_DIRECTORY ${_demo})
        ADD_SUBDIRECTORY(${_demo})
    ENDIF()
ENDFOREACH()
