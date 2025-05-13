include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG 1da23a3e8119ec5cce4f9388e91b065e20bf06f5 # v2.4.12
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(doctest)

find_package(doctest CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${doctest_SOURCE_DIR}/scripts/cmake")

include(doctest)

add_executable(unit_tests
        enum_cast.test.cpp
        float_cast.test.cpp
        narrow_cast.test.cpp
        sign_cast.test.cpp
)
target_link_libraries(unit_tests PRIVATE better_casts doctest::doctest_with_main)
doctest_discover_tests(unit_tests)
