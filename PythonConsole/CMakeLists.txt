cmake_minimum_required(VERSION 3.5)

project(PythonConsole VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

getSrcFiles() # Utility.cmake

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)


target_compile_definitions(${PROJECT_NAME} PRIVATE
    PYTHON_CONSOLE_LIBRARY
)


setOutputDir(${PROJECT_NAME}) # Utility.cmake
linkPybind11ForEmbed(${PROJECT_NAME}) # Utility.cmake
linkPython(${PROJECT_NAME}) # Utility.cmake
