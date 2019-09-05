cmake_minimum_required(VERSION 3.5.1)

# Enable C++14
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_definitions(-Wall -O2)

project(image_stitching)

# Useful macros
include(cmake/util.cmake)

# OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# OpemMP for parallel computing
find_package(OpenMP)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
)

if (APPLE)
    # Fix linking on macOS 10.14+. See https://stackoverflow.com/questions/54068035
    link_directories(/usr/local/lib)
    include_directories(/usr/local/include)
endif ()

add_library(
    ${PROJECT_NAME}
    src/stitcher.cpp
    src/video_provider.cpp
)
target_link_libraries(
    ${PROJECT_NAME}
    ${OpenCV_LIBS}
    OpenMP::OpenMP_CXX
)

add_simple_test(test_images)
add_simple_test(test_videos)
add_simple_test(test_streams)
add_simple_test(test_stitching_detailed_origianl)
