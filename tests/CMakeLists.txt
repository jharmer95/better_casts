enable_testing()

include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG 1da23a3e8119ec5cce4f9388e91b065e20bf06f5 # v2.4.12
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(doctest)

find_package(doctest CONFIG REQUIRED)

add_executable(test_enum_cast enum_cast.test.cpp)
target_link_libraries(test_enum_cast PRIVATE better_casts doctest::doctest_with_main)
add_test(NAME "enum_cast" COMMAND test_enum_cast)

add_executable(test_float_cast float_cast.test.cpp)
target_link_libraries(test_float_cast PRIVATE better_casts doctest::doctest_with_main)
add_test(NAME "float_cast" COMMAND test_float_cast)

add_executable(test_narrow_cast narrow_cast.test.cpp)
target_link_libraries(test_narrow_cast PRIVATE better_casts doctest::doctest_with_main)
add_test(NAME "narrow_cast" COMMAND test_narrow_cast)

add_executable(test_sign_cast sign_cast.test.cpp)
target_link_libraries(test_sign_cast PRIVATE better_casts doctest::doctest_with_main)
add_test(NAME "sign_cast" COMMAND test_sign_cast)
