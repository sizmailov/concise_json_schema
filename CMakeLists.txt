cmake_minimum_required(VERSION 3.1.3)
project(concise_json_schema)

set(CMAKE_CXX_STANDARD 14)
enable_testing()



add_subdirectory(external/console_style)
include_directories(${console_style_INCLUDE_DIRS})
include_directories(external/experimental_variant)

add_subdirectory(external/gtest)
include_directories(external/gtest/googletest/include)

include_directories(include)

add_subdirectory(test)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp )

add_library(${PROJECT_NAME} ${SOURCE_FILES})

add_executable(${PROJECT_NAME}_example example.cpp)
target_link_libraries(${PROJECT_NAME}_example ${PROJECT_NAME})

set(${PROJECT_NAME}_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include
        ${console_style_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/external/experimental_variant
        ${PROJECT_SOURCE_DIR}/gtest/googletest/include
        CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)


target_link_libraries(${PROJECT_NAME} console_style)
