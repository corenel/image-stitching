macro(add_simple_test name)
    add_executable(
        ${name}
        tests/${name}.cpp
    )
    target_link_libraries(
        ${name}
        ${PROJECT_NAME}
    )
endmacro()

macro(add_python_binding name)
    execute_process(COMMAND
        "${SWIG_EXECUTABLE}" "-c++" "-python"
        ${CMAKE_SWIG_FLAGS}
        "-MM" "${CMAKE_CURRENT_SOURCE_DIR}/bindings/${name}.i"
        OUTPUT_VARIABLE ${name}_DEPENDENCIES)

    # Remove the first line
    string(REGEX REPLACE "^.+: +\\\\\n +" ""
        ${name}_DEPENDENCIES "${${name}_DEPENDENCIES}")
    # Clean the end of each line
    string(REGEX REPLACE " +(\\\\)?\n" "\n"
        ${name}_DEPENDENCIES "${${name}_DEPENDENCIES}")
    # Clean beginning of each line
    string(REGEX REPLACE "\n +" "\n"
        ${name}_DEPENDENCIES "${${name}_DEPENDENCIES}")
    # Clean paths
    string(REGEX REPLACE "\\\\\\\\" "/"
        ${name}_DEPENDENCIES "${${name}_DEPENDENCIES}")
    string(REGEX REPLACE "\n" ";"
        ${name}_DEPENDENCIES "${${name}_DEPENDENCIES}")

    set(SWIG_MODULE_${name}_EXTRA_DEPS ${${name}_DEPENDENCIES})
    set_source_files_properties(bindings/${name}.i PROPERTIES CPLUSPLUS ON)
    swig_add_module(${name} python bindings/${name}.i
        # include/${name}.hpp
        bindings/wrapper.hpp
        )
    swig_link_libraries(${name}
        ${OpenCV_LIBS}
        ${PYTHON_LIBRARIES}
        ${PROJECT_NAME})
endmacro()