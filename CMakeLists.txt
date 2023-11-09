cmake_minimum_required(VERSION 3.5)

project(QtCpp-PythonConsole VERSION 0.1 LANGUAGES CXX)

include(Utility.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

set(MY_PYTHON_ROOT C:/Users/funne/AppData/Local/Programs/Python/Python311)

my_get_source_files()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${SOURCES}
        ${HEADERS}
        ${FORMS}
        resource.qrc
        README.md
    )
endif()

target_link_directories(${PROJECT_NAME} PRIVATE
    ${MY_PYTHON_ROOT}/libs
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    $<$<CONFIG:Debug>:python311_d.lib>
    $<$<CONFIG:Release>:python311.lib>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${MY_PYTHON_ROOT}/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

my_link_pybind11_embed(${PROJECT_NAME})
