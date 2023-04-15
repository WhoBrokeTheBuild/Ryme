# ConfigureFileList.cmake
#
# This will run CONFIGURE_FILE for each file in ${_input_list}, and store them in their 
# relative location under the binary directory, with the last extension stripped.
# The resulting list of filenames will be stored in ${_output_list}.
#
# For example:
#   ${CMAKE_CURRENT_SOURCE_DIR}/Source/Config.h.in
# will be configured and stored in 
#   ${CMAKE_BINARY_SOURCE_DIR}/Source/config.h
#

macro(ryme_configure_file_list _input_list _output_list)
    foreach(_input ${_input_list})

        # Replace leading source directory in path with binary directory
        string(REPLACE 
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
            _output
            ${_input}
        )

        # Remove the ".in"
        string(REGEX MATCH "^(.*)\\.[^.]*$" _output ${_output})
        set(_output ${CMAKE_MATCH_1})

        configure_file(${_input} ${_output})

        list(APPEND ${_output_list} ${_output})
    endforeach()
endmacro()