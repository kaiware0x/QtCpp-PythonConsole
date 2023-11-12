project(mymod LANGUAGES CXX)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(${PROJECT_NAME}
    main.cpp
)

my_link_python(${PROJECT_NAME})

my_set_output_bin_dir(${PROJECT_NAME})
