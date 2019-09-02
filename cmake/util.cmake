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