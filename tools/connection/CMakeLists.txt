add_executable(connection main.cpp)
set_target_properties(connection
    PROPERTIES
        CXX_STANDARD 20
        CMAKE_CXX_EXTENSIONS OFF)

target_link_libraries(connection
    PRIVATE
        qbind)

# # Only need NOSSL as this is not a client
# set(KDB_ROOT "${PROJECT_SOURCE_DIR}/vendored/kx")
# find_package(KDB REQUIRED COMPONENTS NOSSL)
# target_link_libraries(playground
#     PUBLIC
#         KDB::KDB)