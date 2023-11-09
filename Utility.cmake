

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
    list(FILTER SOURCES   EXCLUDE REGEX "^deprecated/.*")
    list(FILTER HEADERS   EXCLUDE REGEX "^deprecated/.*")
    list(FILTER FORMS     EXCLUDE REGEX "^deprecated/.*")
    list(FILTER RESOURCES EXCLUDE REGEX "^deprecated/.*")
endmacro()



function(my_link_python target)
    find_package(Python COMPONENTS Interpreter Development)

    message(STATUS "Python_FOUND: ${Python_FOUND}")
    message(STATUS "Python_LIBRARY_DIRS: ${Python_LIBRARY_DIRS}")
    message(STATUS "Python_LIBRARIES: ${Python_LIBRARIES}")
    message(STATUS "Python_INCLUDE_DIR: ${Python_INCLUDE_DIR}")

    target_link_libraries(${target} PRIVATE
        ${Python_LIBRARIES}
    )
    target_include_directories(${target} PRIVATE
        ${Python_INCLUDE_DIR}
    )
endfunction()


# You must this argument to cmake configure command if you installed pybind11 by vcpkg.
# -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
function(my_link_pybind11_embed target)
    find_package(pybind11 CONFIG REQUIRED)
    target_link_libraries(${target} PRIVATE pybind11::embed)
endfunction()
