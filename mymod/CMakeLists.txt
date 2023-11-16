project(mymod LANGUAGES CXX)

findPybind11ForExtend()

getSrcFiles()

pybind11_add_module(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
#    ${FORMS}
#    ${RESOURCES}
)

linkPython(${PROJECT_NAME})
setOutputDir(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" # .pyd destination
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
