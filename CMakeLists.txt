cmake_minimum_required(VERSION 3.20)

# Common CMake project
project(section2 LANGUAGES CXX)

# Use C++20 standard, disabled GNU extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add export compile_command.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable testing via CTest
enable_testing()

# Add homeworks
add_subdirectory(homework_04)
add_subdirectory(homework_05)
add_subdirectory(homework_06)
add_subdirectory(homework_07)

# Add demo
add_subdirectory(demos/lesson_2_4/debug_probe)
