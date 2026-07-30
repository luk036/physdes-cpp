cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

project(Recti LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies (all installed by the parent build) ----

include(../cmake/CPM.cmake)

# Find dependencies first — LdsGenConfig.cmake references fmt::fmt, spdlog::spdlog etc.
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(LdsGen REQUIRED)
find_package(${PROJECT_NAME} REQUIRED)

set(SPECIFIC_LIBS LdsGen::LdsGen fmt::fmt Microsoft.GSL::GSL spdlog::spdlog)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../standalone ${CMAKE_BINARY_DIR}/test_installed)
