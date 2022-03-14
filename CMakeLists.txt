# This is when export changed
cmake_minimum_required(VERSION 3.15)

project(
    qbind
    LANGUAGES CXX
)

# Make find modules available
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(include)
add_subdirectory(tests)
add_subdirectory(tools)