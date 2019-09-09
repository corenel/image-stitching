cmake_minimum_required(VERSION 3.5.1)

# Enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_definitions(-Wall -O2)

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
# Find OpenMP
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
add_simple_test(test_streams)
add_simple_test(test_gst)
# add_simple_test(test_stitching_detailed_origianl)
