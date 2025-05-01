cmake_minimum_required(VERSION 3.26)
project(better_casts VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FLOAT_OPS
        Ceiling
        Floor
        Round
        Truncate
)

option(DEFAULT_FLOAT_CAST_OP "Default float cast operation" Truncate)
option(WERROR "Treat all warnings as errors" OFF)

if (NOT DEFAULT_FLOAT_CAST_OP IN_LIST FLOAT_OPS)
    message(FATAL_ERROR "Invalid default float cast operation: ${DEFAULT_FLOAT_CAST_OP}. Must be one of: ${FLOAT_OPS}.")
endif ()

if(WIN32)
    set(TARGET_WINDOWS TRUE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CXX_MSVC TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CXX_MINGW TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "GNU")
            set(CXX_CLANG TRUE)
        else()
            set(CXX_CLANG_CL TRUE)
            add_compile_options(/EHsc)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CXX_INTEL TRUE)
    else()
        message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for Windows are: [MSVC, GNU, Clang, Intel]")
    endif()
elseif(UNIX AND NOT APPLE)
    set(TARGET_LINUX TRUE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CXX_GCC TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CXX_CLANG TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CXX_INTEL TRUE)
    else()
        message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for UNIX are: [GNU, Clang, Intel]")
    endif()
elseif(APPLE)
    set(TARGET_APPLE TRUE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CXX_GCC TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CXX_CLANG TRUE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(CXX_INTEL TRUE)
    else()
        message(FATAL_ERROR "Invalid compiler: ${CMAKE_CXX_COMPILER_ID}, supported compilers for macOS are: [GNU, Clang, Intel]")
    endif()
endif()

if(CXX_MSVC)
    set(FULL_WARNING
            /W4
            /experimental:external
            /external:W0
            /external:anglebrackets
            /permissive-
            /wd4619
            /w14242
            /w14254
            /w14263
            /w14265
            /w14287
            /we4289
            /w14296
            /w14311
            /w14545
            /w14546
            /w14547
            /w14549
            /w14555
            /w14640
            /w14826
            /w14905
            /w14906
            /w14928
            /we4834
            /Zc:__cplusplus
            /bigobj
            -D_WIN32_WINNT=0x0A00)
    set(MIN_WARNING
            /W1
            /experimental:external
            /external:W0
            /external:anglebrackets
            /wd4619
            /Zc:__cplusplus
            /bigobj
            -D_WIN32_WINNT=0x0A00)
    if(WERROR)
        list(APPEND FULL_WARNING /WX)
        list(APPEND MIN_WARNING /WX)
    endif()
elseif(CXX_GCC OR CXX_MINGW OR CXX_INTEL)
    set(FULL_WARNING
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Weffc++
            -Wuseless-cast
            -Wlogical-op
            -Wduplicated-branches
            -Wduplicated-cond
            -Wmisleading-indentation
            -Wmissing-field-initializers)
    set(MIN_WARNING -Wall)
    if(CXX_MINGW)
        list(APPEND FULL_WARNING -Wa,-mbig-obj)
        list(APPEND MIN_WARNING -Wa,-mbig-obj)
        set(USE_PCH OFF)
    endif()
    if(WERROR)
        list(APPEND FULL_WARNING -Werror)
        list(APPEND MIN_WARNING -Werror)
    endif()
elseif(CXX_CLANG OR CXX_CLANG_CL)
    set(FULL_WARNING
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++98-c++11-compat
            -Wno-c++98-c++11-compat-binary-literal
            -Wno-c++98-c++11-c++14-compat
            -Wno-padded
            -Wno-exit-time-destructors
            -Wno-missing-prototypes
            -Wno-documentation
            -Wno-documentation-unknown-command
            -Wno-weak-vtables
            -Wno-reserved-id-macro
            -Wno-missing-braces
            -Wno-covered-switch-default
            -Wno-unused-template
            -Wno-c++20-attribute-extensions
            -Wno-assume)
    set(MIN_WARNING -Wall)
    if(CXX_CLANG_CL)
        list(APPEND FULL_WARNING
                /bigobj
                -D_WIN32_WINNT=0x0A00)
        list(APPEND MIN_WARNING
                /bigobj
                -D_WIN32_WINNT=0x0A00)
    endif()
    if(WERROR)
        list(APPEND FULL_WARNING -Werror)
        list(APPEND MIN_WARNING -Werror)
    endif()
endif()

add_library(better_casts INTERFACE better_casts.hpp)
target_compile_options(better_casts INTERFACE ${FULL_WARNING})

if (DEFAULT_FLOAT_CAST_OP STREQUAL "Ceiling")
    target_compile_definitions(better_casts INTERFACE DEFAULT_FLOAT_CAST_OP=1)
elseif (DEFAULT_FLOAT_CAST_OP STREQUAL "Floor")
    target_compile_definitions(better_casts INTERFACE DEFAULT_FLOAT_CAST_OP=2)
elseif (DEFAULT_FLOAT_CAST_OP STREQUAL "Round")
    target_compile_definitions(better_casts INTERFACE DEFAULT_FLOAT_CAST_OP=3)
else ()
    target_compile_definitions(better_casts INTERFACE DEFAULT_FLOAT_CAST_OP=4)
endif ()

add_executable(tester tester.cpp)
target_link_libraries(tester PRIVATE better_casts)
