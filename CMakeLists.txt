cmake_minimum_required(VERSION 3.1.3)
project(concise_json_schema)

set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCE_FILES src/*.cpp)


add_subdirectory(external/console_style)
include_directories(${console_style_INCLUDE_DIRS})
include_directories(external/experimental_variant)



include_directories(include)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp )

add_library(${PROJECT_NAME} ${SOURCE_FILES})

add_executable(${PROJECT_NAME}_example example.cpp)
target_link_libraries(${PROJECT_NAME}_example ${PROJECT_NAME})

set(${PROJECT_NAME}_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include
        ${console_style_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/external/experimental_variant
        CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)


target_link_libraries(${PROJECT_NAME} console_style)
