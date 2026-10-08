cmake_minimum_required(VERSION 3.20)
project(CVM_Analysis VERSION 3.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warning flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

enable_testing()

# Case 02 test executable
add_executable(test_case02 tests/unit/test_case02.cpp)
add_test(NAME test_case02 COMMAND test_case02)
