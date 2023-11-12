

# my_get_source_filesを実行したあとに呼ぶ。
macro(my_exclude_source_files DIR_NAME)
    # deprecatedフォルダ以下のソースは除外する
    list(FILTER SOURCES   EXCLUDE REGEX "^${DIR_NAME}/.*")
    list(FILTER HEADERS   EXCLUDE REGEX "^${DIR_NAME}/.*")
    list(FILTER FORMS     EXCLUDE REGEX "^${DIR_NAME}/.*")
    list(FILTER RESOURCES EXCLUDE REGEX "^${DIR_NAME}/.*")
endmacro()


# @brief: macro呼び出し元のCMakeLists.txt直下のソールファイルパスを取得する.
macro(my_get_source_files)
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
    my_exclude_source_files(deprecated)
endmacro()

function(my_set_output_bin_dir target)
    set_target_properties(${target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endfunction()


function(my_link_python target)
    find_package(Python COMPONENTS Interpreter Development REQUIRED)

    message(STATUS "Python_FOUND:           ${Python_FOUND}")
    message(STATUS "PYTHON_VERSION:         ${PYTHON_VERSION}")
    message(STATUS "PYTHON_EXECUTABLE:      ${PYTHON_EXECUTABLE}")
    message(STATUS "Python_LIBRARY_DIRS:    ${Python_LIBRARY_DIRS}")
    message(STATUS "Python_LIBRARIES:       ${Python_LIBRARIES}")
    message(STATUS "PYTHON_INCLUDE_DIRS:    ${PYTHON_INCLUDE_DIRS}")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(GET Python_LIBRARIES 3 MY_PYTHON_LIB_PATH)
    else()
        list(GET Python_LIBRARIES 1 MY_PYTHON_LIB_PATH)
    endif()
    message(STATUS "MY_PYTHON_LIB_PATH: ${MY_PYTHON_LIB_PATH}")

    # Get file name e.g. "python312" or "python312_d"
    cmake_path(GET MY_PYTHON_LIB_PATH STEM MY_PYTHON_LIB_NAME)
    message(STATUS "MY_PYTHON_LIB_NAME: ${MY_PYTHON_LIB_NAME}")

    # Get python executable file directory e.g. "C:/Python/Python312/"
    cmake_path(GET PYTHON_EXECUTABLE PARENT_PATH MY_PYTHON_BIN_DIR)
    message(STATUS "MY_PYTHON_BIN_DIR: ${MY_PYTHON_BIN_DIR}")

    target_link_directories(${target} PRIVATE
        ${Python_LIBRARY_DIRS}
    )
    target_link_libraries(${target} PRIVATE
        ${MY_PYTHON_LIB_NAME}
    )
    target_include_directories(${target} PRIVATE
        ${PYTHON_INCLUDE_DIRS}
    )

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${MY_PYTHON_BIN_DIR}/${MY_PYTHON_LIB_NAME}.dll"
            $<TARGET_FILE_DIR:${target}>
    )
endfunction()


# You must this argument to cmake configure command if you installed pybind11 by vcpkg.
# -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
function(my_link_pybind11_embed target)
    find_package(pybind11 CONFIG REQUIRED)
    target_link_libraries(${target} PRIVATE pybind11::embed)
endfunction()

function(my_link_pybind11_extend)
    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)
endfunction()

function(my_copy_python_shared_lib target)
    message(STATUS "my_copy_python_shared_lib")
    message(STATUS "PYTHON_EXECUTABLE:      ${PYTHON_EXECUTABLE}")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(GET Python_LIBRARIES 3 MY_PYTHON_LIB_PATH)
    else()
        list(GET Python_LIBRARIES 1 MY_PYTHON_LIB_PATH)
    endif()
    message(STATUS "MY_PYTHON_LIB_PATH: ${MY_PYTHON_LIB_PATH}")

    # Get file name e.g. "python312" or "python312_d"
    cmake_path(GET MY_PYTHON_LIB_PATH STEM MY_PYTHON_LIB_NAME)
    message(STATUS "MY_PYTHON_LIB_NAME: ${MY_PYTHON_LIB_NAME}")

    # Get python executable file directory e.g. "C:/Python/Python312/"
    cmake_path(GET PYTHON_EXECUTABLE PARENT_PATH MY_PYTHON_BIN_DIR)
    message(STATUS "MY_PYTHON_BIN_DIR: ${MY_PYTHON_BIN_DIR}")

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${MY_PYTHON_BIN_DIR}/${MY_PYTHON_LIB_NAME}.dll"
            $<TARGET_FILE_DIR:${target}>
    )
endfunction()
