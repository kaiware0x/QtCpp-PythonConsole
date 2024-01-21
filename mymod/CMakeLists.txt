project(mymod LANGUAGES CXX)

findPybind11ForExtend()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

getSrcFiles()

pybind11_add_module(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
#    ${FORMS}
#    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

linkPython(${PROJECT_NAME})
setOutputDir(${PROJECT_NAME})

linkOpenCASCADE(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" # .pyd destination
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
