cmake_minimum_required(VERSION 3.20)
project(ezcli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EZCLI_BUILD_TESTS      "Build unit tests"                      ON)
option(EZCLI_BUILD_BENCHMARKS "Build benchmarks"                      OFF)
option(EZCLI_SANITIZE         "Build with ASan (+ UBSan on GCC/Clang)" OFF)
option(EZCLI_BUILD_FUZZ       "Build libFuzzer target (Clang only)"   OFF)

if(EZCLI_BUILD_FUZZ AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR
        "EZCLI_BUILD_FUZZ requires Clang. "
        "Re-run cmake with -DCMAKE_CXX_COMPILER=clang++")
endif()

if(EZCLI_SANITIZE)
    if(MSVC)
        add_compile_options(/fsanitize=address /Zi)
        foreach(flag CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG)
            string(REPLACE "/RTC1" "" ${flag} "${${flag}}")
        endforeach()
        add_link_options(/INCREMENTAL:NO)
    else()
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    endif()
elseif(EZCLI_BUILD_FUZZ)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(ezcli ez_cli.cpp)
target_include_directories(ezcli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(EZCLI_BUILD_TESTS)
    target_compile_definitions(ezcli PRIVATE EZ_CLI_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(EZCLI_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(EZCLI_BUILD_FUZZ)
    add_subdirectory(fuzz)
endif()
