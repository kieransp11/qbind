add_library(qbind.kdb.tests SHARED
    add.cpp)

set_target_properties(qbind.kdb.tests
    PROPERTIES
        CXX_STANDARD 20
        CMAKE_CXX_EXTENSIONS OFF)

target_link_libraries(qbind.kdb.tests
    PRIVATE
        qbind)

find_package(KDB REQUIRED COMPONENTS NOSSL)
target_link_libraries(qbind.kdb.tests
    PUBLIC
        KDB::KDB)
