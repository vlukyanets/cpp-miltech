cmake_minimum_required(VERSION 3.20)

# Use C++20 standard, disabled GNU extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ballistics static lib project
project(ballistics CXX)
add_library(ballistics src/ballistics.cpp)
target_include_directories(ballistics PUBLIC include)

# Ballistics CLI executable project
project(ballistics_cli CXX)
add_executable(ballistics_cli src/main.cpp)
target_link_libraries(ballistics_cli PRIVATE ballistics)

if(NOT CMAKE_CROSSCOMPILING)
    # Ballistics tests
    include(FetchContent)
    include(GoogleTest)
    FetchContent_Declare(
        googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
                       DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
    project(ballistics_tests CXX)
    add_executable(ballistics_tests tests/ballistics_tests.cpp tests/ballistics_cli_tests.cpp)
    target_link_libraries(ballistics_tests PRIVATE ballistics GTest::gtest_main)
    target_compile_definitions(ballistics_tests
                               PRIVATE TESTDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    gtest_discover_tests(ballistics_tests DISCOVERY_MODE PRE_TEST)
endif()
