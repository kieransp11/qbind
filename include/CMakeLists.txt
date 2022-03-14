# Header only library
add_library(qbind INTERFACE)

target_include_directories(qbind INTERFACE "")

set_target_properties(qbind
    PROPERTIES
        CXX_STANDARD 20
        CMAKE_CXX_EXTENSIONS OFF)

# Require header only libraries
find_package(Boost REQUIRED)
target_link_libraries(qbind
    INTERFACE
        Boost::boost)

# Only need NOSSL as this is not a client
set(KDB_ROOT "${PROJECT_SOURCE_DIR}/vendored/kx")
find_package(KDB REQUIRED COMPONENTS NOSSL)
target_link_libraries(qbind
    INTERFACE
        KDB::KDB)
