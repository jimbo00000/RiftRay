# Generate a header file with all shaders initialzed as string literals.

#
# Auto-generate hardcoded shader headers in python.
# TODO: This was a pain to get to work and likely not the best solution...
#
set (python_cmd "python")
set (python_arg "tools/hardcode_shaders.py")
message(STATUS "\n\nGenerating shader include file...")
execute_process(COMMAND ${python_cmd} ${python_arg}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE python_result
    OUTPUT_VARIABLE python_ver)
message(STATUS "${python_result}: ${python_ver}\n\n")
INCLUDE_DIRECTORIES("autogen/")
