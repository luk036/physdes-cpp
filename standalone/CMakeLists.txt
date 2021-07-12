cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(PhysDesCppStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 2.2.1
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)
CPMAddPackage(NAME PhysDesCpp SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
CPMAddPackage("gh:microsoft/GSL@3.1.0")

find_package(Boost REQUIRED)
if(Boost_FOUND)
    message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
    include_directories(${BOOST_INCLUDE_DIRS})
    add_definitions(${Boost_DEFINITIONS})
endif()

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(PhysDesCppStandalone ${sources})

set_target_properties(PhysDesCppStandalone PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "PhysDesCpp")

target_link_libraries(PhysDesCppStandalone PhysDesCpp::PhysDesCpp cxxopts PRIVATE GSL)
