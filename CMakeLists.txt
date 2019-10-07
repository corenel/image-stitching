cmake_minimum_required(VERSION 3.5.1)

# Enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_definitions(-Wall -Wno-dev -g)

project(image_stitching)

# Useful macros
include(cmake/util.cmake)

if (APPLE)
    # Fix linking on macOS 10.14+. See https://stackoverflow.com/questions/54068035
    link_directories(/usr/local/lib)
    include_directories(/usr/local/include)
endif ()

# OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# OpemMP for parallel computing
OPTION(USE_OPENMP "Use OpenMP to enamble <omp.h>" ON)
if (APPLE)
    SET(USE_OPENMP OFF)
endif ()
if (USE_OPENMP)
    find_package(OpenMP REQUIRED)
    ADD_DEFINITIONS(-DUSE_OPENMP)
    message(STATUS "OpenMP enabled")
endif (USE_OPENMP)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
)

add_library(
    ${PROJECT_NAME}
    src/stitcher.cpp
    src/warper.cpp
    src/video_provider.cpp
    src/stream_provider.cpp
    src/stream_writer.cpp
)
target_link_libraries(
    ${PROJECT_NAME}
    ${OpenCV_LIBS}
)
if (USE_OPENMP)
    target_link_libraries(
        ${PROJECT_NAME}
        OpenMP::OpenMP_CXX
    )
endif (USE_OPENMP)

#add_simple_test(test_images)
#add_simple_test(test_videos)
#add_simple_test(test_streams)
add_simple_test(test_gst)
add_simple_test(test_stitching_detailed_origianl)
add_simple_test(test_warp_perspective)

OPTION(BUILD_PYTHON_BINDINGS "Build bindings for Python" OFF)
if (BUILD_PYTHON_BINDINGS)
    # find swig
    find_package(SWIG REQUIRED)
    set(CMAKE_SWIG_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/../opencv-swig/lib/")
    include(${SWIG_USE_FILE})
    # find python library
    set(TEST_PYTHON_VERSION "3"
        CACHE STRING "Python version used on tests.")
    set_property(CACHE TEST_PYTHON_VERSION PROPERTY STRINGS "3" "2")
    if (TEST_PYTHON_VERSION EQUAL 2)
        find_package(PythonLibs 2 REQUIRED)
    else ()
        find_package(PythonLibs 3 REQUIRED)
    endif ()
    message(STATUS "Python: ${TEST_PYTHON_VERSION}")
    # include
    include_directories(
        bindings
        ${PYTHON_INCLUDE_DIRS}
    )
    get_property(_include_directories DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
    foreach (_include_directory IN LISTS _include_directories)
        list(APPEND CMAKE_SWIG_FLAGS -I${_include_directory})
    endforeach ()
    # build bindings
    add_python_binding(image_stitching)
    # add_python_binding(stitcher)
    # add_python_binding(stream_provider)
    # add_python_binding(stream_writer)
endif (BUILD_PYTHON_BINDINGS)
