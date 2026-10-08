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

# Case 03 test executable
add_executable(test_case03 tests/unit/test_case03.cpp)
add_test(NAME test_case03 COMMAND test_case03)

# Case 04 test executable
add_executable(test_case04 tests/unit/test_case04.cpp)
add_test(NAME test_case04 COMMAND test_case04)

# Case 05 test executable
add_executable(test_case05 tests/unit/test_case05.cpp)
add_test(NAME test_case05 COMMAND test_case05)

# Case 06 test executable
add_executable(test_case06 tests/unit/test_case06.cpp)
add_test(NAME test_case06 COMMAND test_case06)

# Case 07 test executable
add_executable(test_case07 tests/unit/test_case07.cpp)
add_test(NAME test_case07 COMMAND test_case07)

# Case 08 test executable
add_executable(test_case08 tests/unit/test_case08.cpp)
add_test(NAME test_case08 COMMAND test_case08)

# Case 09 test executable
add_executable(test_case09 tests/unit/test_case09.cpp)
add_test(NAME test_case09 COMMAND test_case09)
