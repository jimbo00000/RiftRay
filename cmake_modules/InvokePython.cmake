# Invokes python on a given string with script name followed by args.

function( INVOKEPYTHON ARGS )
    # @TODO This was a pain to get to work and likely not the best solution...
    set (python_cmd "python")
    message(STATUS "Invoking ${python_cmd} ${ARGS}:" )
    execute_process(COMMAND ${python_cmd} ${ARGS}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE python_result
        OUTPUT_VARIABLE python_output)
    
    # @TODO check the output of the execute_process and check for success.
    # This was difficult to find the docs for the Windows version of this, which prints
    # "The system cannot find the file specified".
    message(STATUS "${python_output}" )
    message(STATUS "${python_result}" )
    
    if(python_result STREQUAL "0")
        message(STATUS "${python_arg} success." )
    else(python_result STREQUAL "0")
        message(STATUS "##################################################################################" )
        message(STATUS "#" )
        message(STATUS "# ${python_cmd} wasn't found. Go to https://www.python.org/download/" )
        message(STATUS "# and download Python 2.7. Once installed, don't forget to add its installation" )
        message(STATUS "# directory to your $PATH. (default C:/Python27/ on Windows)" )
        message(STATUS "# On Windows 7 & 8 this can be found by right-clicking My Computer," )
        message(STATUS "# Advanced System Settings, Environment Variables..." )
        message(STATUS "#" )
        message(STATUS "##################################################################################" )
    endif(python_result STREQUAL "0")
endfunction( INVOKEPYTHON )
