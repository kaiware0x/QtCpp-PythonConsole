

# getSrcFilesを実行したあとに呼ぶ。
macro(excludeSrcDir DIR_NAME)
    # deprecatedフォルダ以下のソースは除外する
    list(FILTER SOURCES   EXCLUDE REGEX "^${DIR_NAME}/.*")
    list(FILTER HEADERS   EXCLUDE REGEX "^${DIR_NAME}/.*")
    list(FILTER FORMS     EXCLUDE REGEX "^${DIR_NAME}/.*")
    list(FILTER RESOURCES EXCLUDE REGEX "^${DIR_NAME}/.*")
endmacro()


# @brief: macro呼び出し元のCMakeLists.txt直下のソールファイルパスを取得する.
macro(getSrcFiles)
    # 直下のソースファイル群を取得する
    file(GLOB_RECURSE SOURCES     RELATIVE ${PROJECT_SOURCE_DIR} *.cpp)
    file(GLOB_RECURSE SOURCES_CXX RELATIVE ${PROJECT_SOURCE_DIR} *.cxx)
    file(GLOB_RECURSE SOURCES_CU  RELATIVE ${PROJECT_SOURCE_DIR} *.cu)
    file(GLOB_RECURSE HEADERS     RELATIVE ${PROJECT_SOURCE_DIR} *.h)
    file(GLOB_RECURSE HEADERS_HPP RELATIVE ${PROJECT_SOURCE_DIR} *.hpp)
    file(GLOB_RECURSE HEADERS_HXX RELATIVE ${PROJECT_SOURCE_DIR} *.hxx)
    file(GLOB_RECURSE HEADERS_CUH RELATIVE ${PROJECT_SOURCE_DIR} *.cuh)
    file(GLOB_RECURSE FORMS       RELATIVE ${PROJECT_SOURCE_DIR} *.ui)
    file(GLOB_RECURSE RESOURCES   RELATIVE ${PROJECT_SOURCE_DIR} *.qrc)

    # .cxxと.hxxをSOURCESとHEADERSに統合
    list(APPEND SOURCES ${SOURCES_CXX})
    list(APPEND SOURCES ${SOURCES_CU})
    list(APPEND HEADERS ${HEADERS_HPP})
    list(APPEND HEADERS ${HEADERS_HXX})
    list(APPEND HEADERS ${HEADERS_CUH})

    # 変数を破棄
    unset(SOURCES_CXX)
    unset(SOURCES_CU)
    unset(HEADERS_HPP)
    unset(HEADERS_HXX)
    unset(HEADERS_CUH)

    # deprecatedフォルダ以下のソースは除外する
    excludeSrcDir(deprecated)
endmacro()


#
function(setOutputDir target)
    set_target_properties(${target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endfunction()


# Get python executable file directory e.g. "C:/Python/Python312/"
function(getPythonExeDir)
    message(STATUS "Call getPythonExeDir()")

    cmake_path(GET PYTHON_EXECUTABLE PARENT_PATH MY_PYTHON_BIN_DIR)
    set(MY_PYTHON_BIN_DIR ${MY_PYTHON_BIN_DIR} PARENT_SCOPE)
    message(STATUS "MY_PYTHON_BIN_DIR: ${MY_PYTHON_BIN_DIR}")
endfunction()


# Get file name e.g. "python312" or "python312_d"
function(getPythonLibName)
    message(STATUS "Call getPythonLibName()")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(GET Python_LIBRARIES 3 MY_PYTHON_LIB_PATH)
    else()
        list(GET Python_LIBRARIES 1 MY_PYTHON_LIB_PATH)
    endif()
    message(STATUS "MY_PYTHON_LIB_PATH: ${MY_PYTHON_LIB_PATH}")

    # Get file name e.g. "python312" or "python312_d"
    cmake_path(GET MY_PYTHON_LIB_PATH STEM MY_PYTHON_LIB_NAME)
    set(MY_PYTHON_LIB_NAME ${MY_PYTHON_LIB_NAME} PARENT_SCOPE) # Set to parent scope
    message(STATUS "MY_PYTHON_LIB_NAME: ${MY_PYTHON_LIB_NAME}")
endfunction()


#
function(linkPython target)
    message(STATUS "Call linkPython(${target})")

    find_package(Python COMPONENTS Interpreter Development REQUIRED)

    message(STATUS "Python_FOUND:           ${Python_FOUND}")
    message(STATUS "PYTHON_VERSION:         ${PYTHON_VERSION}")
    message(STATUS "PYTHON_EXECUTABLE:      ${PYTHON_EXECUTABLE}")
    message(STATUS "Python_LIBRARY_DIRS:    ${Python_LIBRARY_DIRS}")
    message(STATUS "Python_LIBRARIES:       ${Python_LIBRARIES}")
    message(STATUS "PYTHON_INCLUDE_DIRS:    ${PYTHON_INCLUDE_DIRS}")


    target_link_directories(${target} PRIVATE
        ${Python_LIBRARY_DIRS}
    )
    getPythonLibName()
    target_link_libraries(${target} PRIVATE
        ${MY_PYTHON_LIB_NAME}
    )
    target_include_directories(${target} PRIVATE
        ${PYTHON_INCLUDE_DIRS}
    )
endfunction()


# Copy python runtime libraries.
function(copyPythonSharedLib target)
    message(STATUS "Call copyPythonSharedLib()")

    getPythonLibName()
    getPythonExeDir()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${MY_PYTHON_BIN_DIR}/${MY_PYTHON_LIB_NAME}.dll"
            $<TARGET_FILE_DIR:${target}>
    )
endfunction()



# You must this argument to cmake configure command if you installed pybind11 by vcpkg.
# -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
function(linkPybind11ForEmbed target)
    find_package(pybind11 CONFIG REQUIRED)
    target_link_libraries(${target} PRIVATE pybind11::embed)
endfunction()


#
function(findPybind11ForExtend)
    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(PYTHON_MODULE_EXTENSION "_d.pyd" CACHE STRING "Built python module custom extension" FORCE)
    else()
        set(PYTHON_MODULE_EXTENSION ".pyd" CACHE STRING "Built python module custom extension" FORCE)
    endif()
endfunction()


##
#function(linkPybind11ForExtend target)
#    # pybind11 method:
#    # pybind11_add_module(${target} src1.cpp)

#    # Python method:
#    # Python_add_library(${target} src2.cpp)

#    target_link_libraries(${target} PRIVATE
#        pybind11::headers
#        pybind11::module
#    )
#    set_target_properties(${target} PROPERTIES
#        INTERPROCEDURAL_OPTIMIZATION ON
#        CXX_VISIBILITY_PRESET ON
#        VISIBILITY_INLINES_HIDDEN ON
#    )
#endfunction()


# You need to install OpenCASCADE using this command:
#   $ vcpkg install opencascade
function(linkOpenCASCADE target)
    # this is heuristically generated, and may not be correct
    find_package(OpenCASCADE CONFIG REQUIRED)

    message(STATUS "OpenCASCADE_FOUND:      ${OpenCASCADE_FOUND}")
    message(STATUS "OpenCASCADE_DIR:        ${OpenCASCADE_DIR}")
    message(STATUS "OpenCASCADE_CONFIG:     ${OpenCASCADE_CONFIG}")
    message(STATUS "OpenCASCADE_INCLUDE_DIR:${OpenCASCADE_INCLUDE_DIR}")
    message(STATUS "OpenCASCADE_LIBRARY_DIR:${OpenCASCADE_LIBRARY_DIR}")
    message(STATUS "OpenCASCADE_BINARY_DIR: ${OpenCASCADE_BINARY_DIR}")
    message(STATUS "OpenCASCADE_LIBRARIES:  ${OpenCASCADE_LIBRARIES}")

    target_include_directories(${target} PRIVATE
        ${OpenCASCADE_INCLUDE_DIR}
    )

    target_link_directories(${target} PRIVATE
        ${OpenCASCADE_LIBRARY_DIR}
    )

    # note: 47 additional targets are not displayed.
    target_link_libraries(${target} PRIVATE
        ${OpenCASCADE_LIBRARIES}
#        TKBO
#        TKBin
#        TKCAF
#        TKCDF
    )
endfunction()
