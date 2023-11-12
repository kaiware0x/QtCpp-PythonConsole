cmake_minimum_required(VERSION 3.5)

project(
  QtCpp-PythonConsole
  VERSION 0.1
  LANGUAGES CXX)

include(Utility.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

my_get_source_files()
my_exclude_source_files(MyPyModule)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
    README.md
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(${PROJECT_NAME})

my_set_output_bin_dir(${PROJECT_NAME})
my_link_pybind11_embed(${PROJECT_NAME})
my_link_python(${PROJECT_NAME})

add_subdirectory(mymod/)
