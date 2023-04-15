# Findpatch.cmake
#
# Finds the patch command
#
# If not found, and using windows, it will download patch.exe from GnuWin32
#
# This will define the following variables
#
#   patch_FOUND
#   patch_COMMAND
#

find_program(
    patch_COMMAND
    NAMES patch
    PATHS
        ${CMAKE_BINARY_DIR}
    PATH_SUFFIXES
        bin
)

if(NOT patch_COMMAND AND WIN32)
    file(DOWNLOAD
        "http://prdownloads.sourceforge.net/gnuwin32/patch-2.5.9-7-bin.zip"
        ${CMAKE_BINARY_DIR}/patch.zip
        EXPECTED_MD5 b9c8b31d62f4b2e4f1887bbb63e8a905
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_BINARY_DIR}/patch.zip
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    find_program(
        patch_COMMAND
        NAMES patch
        PATHS
            ${CMAKE_BINARY_DIR}
        PATH_SUFFIXES
            bin
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    patch
    REQUIRED_VARS 
        patch_COMMAND
)