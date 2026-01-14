# this script adds all subprojects to a single build to allow IDEs understand the full project
# structure.

cmake_minimum_required(VERSION 3.14...3.22)

project(BuildAll LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../cmake/CPM.cmake)
include(../cmake/tools.cmake)

# needed to generate test target
enable_testing()

CPMAddPackage(
  NAME googlebenchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.8.3 # Use a specific version
  OPTIONS "BENCHMARK_ENABLE_TESTING OFF" # Disable benchmark's own tests
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../standalone ${CMAKE_BINARY_DIR}/standalone)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../test ${CMAKE_BINARY_DIR}/test)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../documentation ${CMAKE_BINARY_DIR}/documentation)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../bench ${CMAKE_BINARY_DIR}/bench)
