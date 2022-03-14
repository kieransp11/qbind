add_executable(playground main.cpp)
set_target_properties(playground
    PROPERTIES
        CXX_STANDARD 20
        CMAKE_CXX_EXTENSIONS OFF)

target_link_libraries(playground
    PRIVATE
        qbind)