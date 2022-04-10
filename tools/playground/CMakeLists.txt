add_executable(playground main.cpp)
set_target_properties(playground
    PROPERTIES
        CXX_STANDARD 20
        CMAKE_CXX_EXTENSIONS OFF)

target_link_libraries(playground
    PRIVATE
        qbind)

# Only need NOSSL as this is not a client
set(KDB_ROOT "${PROJECT_SOURCE_DIR}/vendored/kx")
find_package(KDB REQUIRED COMPONENTS NOSSL)
target_link_libraries(playground
    PUBLIC
        KDB::KDB)