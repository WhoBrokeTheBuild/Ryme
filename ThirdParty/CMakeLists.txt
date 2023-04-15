cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

# Determine if this file has been included by ADD_SUBDIRECTORY or run directly from cmake
# If the file was included, it will then run a sub-cmake in order to configure and build
# the required dependencies during the main project's configure step.
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

    set(_source_dir ${CMAKE_SOURCE_DIR}/ThirdParty)
    set(_binary_dir ${CMAKE_BINARY_DIR}/ThirdParty)

    # Set *_ROOT variables for config-based packages
    set(glm_ROOT ${_binary_dir}/glm/cmake PARENT_SCOPE)
    set(fmt_ROOT ${_binary_dir}/fmt PARENT_SCOPE)
    set(SDL2_ROOT ${_binary_dir}/SDL2 PARENT_SCOPE)
    set(pybind11_ROOT ${_binary_dir}/pybind11 PARENT_SCOPE)
    set(nlohmann_json_ROOT ${_binary_dir}/nlohmann_json PARENT_SCOPE)

    # Set CMAKE_PREFIX_PATH for find-based packages
    list(APPEND CMAKE_PREFIX_PATH
        ${_binary_dir}/GTest
        ${_binary_dir}/spirv_cross
    )
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)

    ###
    ### Configure
    ###

    file(MAKE_DIRECTORY ${_source_dir})

    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" 
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            ${_source_dir}
        WORKING_DIRECTORY ${_binary_dir}
        RESULT_VARIABLE _result
    )
    if(_result)
        MESSAGE(SEND_ERROR "${_result}")
        MESSAGE(FATAL_ERROR "Failed to configure ThirdParty projects")
    endif()

    ###
    ### Build
    ###

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${_binary_dir}
        RESULT_VARIABLE _result
    )
    if(_result)
        MESSAGE(SEND_ERROR "${_result}")
        MESSAGE(FATAL_ERROR "Failed to build ThirdParty projects")
    endif()

ELSE()

    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../CMake")
    
    include(ExternalProject)

    project(ThirdParty)

    find_package(patch REQUIRED)

    ###
    ### glm
    ###

    set(glm_ROOT       ${CMAKE_BINARY_DIR}/glm/cmake)
    set(glm_SOURCE_DIR ${CMAKE_BINARY_DIR}/glm)

    find_package(glm CONFIG QUIET)

    if(NOT glm_FOUND OR NOT TARGET glm::glm)
        ExternalProject_Add(
            ThirdParty_glm
            URL                 "https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip"
            URL_HASH            MD5=69895110052f0d711c9c54fbf385f6f5
            SOURCE_DIR          ${glm_SOURCE_DIR}
            CONFIGURE_COMMAND   ""
            BUILD_COMMAND       ""
            INSTALL_COMMAND     ""
            UPDATE_COMMAND      ""
        )
    endif()
    
    ###
    ### fmt
    ###

    set(fmt_ROOT       ${CMAKE_BINARY_DIR}/fmt)
    set(fmt_SOURCE_DIR ${CMAKE_BINARY_DIR}/fmt_src)
    set(fmt_BINARY_DIR ${CMAKE_BINARY_DIR}/fmt_bin)

    find_package(fmt 8.0.0 CONFIG QUIET)

    if(NOT fmt_FOUND)
        ExternalProject_Add(
            ThirdParty_fmt
            URL                 "https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.zip"
            URL_HASH            MD5=e6754011ff56bfc37631fcc90961e377
            SOURCE_DIR          ${fmt_SOURCE_DIR}
            BINARY_DIR          ${fmt_BINARY_DIR}
            CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${fmt_ROOT}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DFMT_DOC=OFF
                                -DFMT_TEST=OFF
                                -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            BUILD_COMMAND       ""
            UPDATE_COMMAND      ""
            TEST_COMMAND        ""
            INSTALL_COMMAND     ${CMAKE_COMMAND} --build . --target install
        )
    endif()

    ###
    ### SDL2
    ###

    set(SDL2_ROOT       ${CMAKE_BINARY_DIR}/SDL2)
    set(SDL2_SOURCE_DIR ${CMAKE_BINARY_DIR}/SDL2_src)
    set(SDL2_BINARY_DIR ${CMAKE_BINARY_DIR}/SDL2_bin)

    find_package(SDL2 2.0.6 CONFIG QUIET)

    if(CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019")
        set(_SDL2_PATCH_COMMAND ${patch_COMMAND} ${SDL2_SOURCE_DIR}/CMakeLists.txt ${CMAKE_SOURCE_DIR}/SDL2_vcruntime.patch )
    endif()

    if(NOT SDL2_FOUND)
        ExternalProject_Add(
            ThirdParty_SDL2
            URL                 "https://www.libsdl.org/release/SDL2-2.0.14.tar.gz"
            URL_HASH            MD5=76ed4e6da9c07bd168b2acd9bfefab1b
            SOURCE_DIR          ${SDL2_SOURCE_DIR}
            BINARY_DIR          ${SDL2_BINARY_DIR}
            CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${SDL2_ROOT}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DBUILD_SHARED_LIBS=ON
            BUILD_COMMAND       ""
            UPDATE_COMMAND      ""
            TEST_COMMAND        ""
            INSTALL_COMMAND     ${CMAKE_COMMAND} --build . --target install
            PATCH_COMMAND       ${_SDL2_PATCH_COMMAND}
        )
    endif()

    ###
    ### pybind11
    ###

    set(pybind11_ROOT       ${CMAKE_BINARY_DIR}/pybind11)
    set(pybind11_SOURCE_DIR ${CMAKE_BINARY_DIR}/pybind11_src)
    set(pybind11_BINARY_DIR ${CMAKE_BINARY_DIR}/pybind11_bin)

    find_package(pybind11 2.8.0 CONFIG QUIET)

    if(NOT pybind11_FOUND)
        ExternalProject_Add(
            ThirdParty_pybind11
            URL                 "https://github.com/pybind/pybind11/archive/refs/tags/v2.10.4.zip"
            URL_HASH            MD5=dd8fa6cf5bc26d26bf25c40f264f63af
            SOURCE_DIR          ${pybind11_SOURCE_DIR}
            BINARY_DIR          ${pybind11_BINARY_DIR}
            CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${pybind11_ROOT}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                -DPYBIND11_TEST=OFF
            BUILD_COMMAND       ""
            UPDATE_COMMAND      ""
            TEST_COMMAND        ""
            INSTALL_COMMAND     ${CMAKE_COMMAND} --build . --target install
        )
    endif()

    ###
    ### nlohmann_json
    ###

    set(nlohmann_json_ROOT       ${CMAKE_BINARY_DIR}/nlohmann_json)
    set(nlohmann_json_SOURCE_DIR ${CMAKE_BINARY_DIR}/nlohmann_json_src)

    find_package(nlohmann_json CONFIG QUIET)

    if(NOT nlohmann_json_FOUND)
        ExternalProject_Add(
            ThirdParty_nlohmann_json
            URL                 "https://github.com/nlohmann/json/archive/refs/tags/v3.10.4.zip"
            URL_HASH            MD5=59c2a25e17b94d612fdb32a1a37378cf
            SOURCE_DIR          ${nlohmann_json_SOURCE_DIR}
            CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${nlohmann_json_ROOT}
                                -DJSON_BuildTests=OFF
            BUILD_COMMAND       ""
            UPDATE_COMMAND      ""
        )
    endif()

    ###
    ### SPIRV-Cross
    ###

    set(spirv_cross_ROOT       ${CMAKE_BINARY_DIR}/spirv_cross)
    set(spirv_cross_SOURCE_DIR ${CMAKE_BINARY_DIR}/spirv_cross_src)
    set(spirv_cross_BINARY_DIR ${CMAKE_BINARY_DIR}/spirv_cross_bin)
    
    list(APPEND CMAKE_PREFIX_PATH
        ${spirv_cross_ROOT}
    )

    find_package(spirv_cross_core CONFIG QUIET)

    if(NOT spirv_cross_core_FOUND)
        ExternalProject_Add(
            ThirdParty_spirv_cross
            # GIT_REPOSITORY      "https://github.com/KhronosGroup/SPIRV-Cross.git"
            URL                 "https://codeload.github.com/KhronosGroup/SPIRV-Cross/zip/refs/tags/sdk-1.3.231.1"
            URL_HASH            MD5=2a9d91e7bbecf00731806e94f1569400
            SOURCE_DIR          ${spirv_cross_SOURCE_DIR}
            BINARY_DIR          ${spirv_cross_BINARY_DIR}
            CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${spirv_cross_ROOT}
                                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            BUILD_COMMAND       ""
            UPDATE_COMMAND      ""
            TEST_COMMAND        ""
            INSTALL_COMMAND     ${CMAKE_COMMAND} --build . --target install
        )
    endif()

    ###
    ### GTest
    ###

    set(GTEST_ROOT       ${CMAKE_BINARY_DIR}/GTest)
    set(GTEST_SOURCE_DIR ${CMAKE_BINARY_DIR}/GTest_src)
    set(GTEST_BINARY_DIR ${CMAKE_BINARY_DIR}/GTest_bin)
    
    list(APPEND CMAKE_PREFIX_PATH
        ${GTEST_ROOT}
    )

    set(GTEST_MSVC_SEARCH "MD")
    find_package(GTest QUIET)

    if(NOT GTEST_FOUND)
        ExternalProject_Add(
            ThirdParty_GTest
            URL                 "https://github.com/google/googletest/archive/release-1.11.0.tar.gz"
            URL_HASH            MD5=e8a8df240b6938bb6384155d4c37d937
            SOURCE_DIR          ${GTEST_SOURCE_DIR}
            BINARY_DIR          ${GTEST_BINARY_DIR}
            CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${GTEST_ROOT}
                                -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                                -DBUILD_SHARED_LIBS=ON
            BUILD_COMMAND       ""
            UPDATE_COMMAND      ""
            TEST_COMMAND        ""
            INSTALL_COMMAND     ${CMAKE_COMMAND} --build . --target install --config ${CMAKE_BUILD_TYPE}
        )
    endif()

endif()


