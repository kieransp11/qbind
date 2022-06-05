# TODO: When Catch3 arrvies use Catch2::Catch2WithMain as described here:
# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
# This means we can delete the dummy main file.

add_executable(qbind.cpp.tests
    main.cpp
    test_kx.cpp
    test_macros.cpp
    test_span.cpp)

set_target_properties(qbind.cpp.tests
    PROPERTIES
        CXX_STANDARD 20
        CMAKE_CXX_EXTENSIONS OFF)

target_link_libraries(qbind.cpp.tests
    PRIVATE
        qbind)

find_package(KDB REQUIRED COMPONENTS NOSSL)
target_link_libraries(qbind.cpp.tests
    PUBLIC
        KDB::KDB)

# Link catch 2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.8
)
FetchContent_MakeAvailable(Catch2)
target_link_libraries(qbind.cpp.tests
    PRIVATE
        Catch2::Catch2)

# Configure CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)
catch_discover_tests(qbind.cpp.tests)

