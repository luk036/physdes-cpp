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

# find_package (Boost REQUIRED)
# if (Boost_FOUND)
#     message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
#     CPMAddPackage(NAME PhysDesCpp SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
# endif (Boost_FOUND)

# boost-cmake currently doesn't tag versions, so we use the according boost version
CPMAddPackage("gh:Orphis/boost-cmake#7f97a08b64bd5d2e53e932ddf80c40544cf45edf@1.71.0")

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(PhysDesCppStandalone ${sources})

set_target_properties(PhysDesCppStandalone PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "PhysDesCpp")

target_link_libraries(PhysDesCppStandalone PhysDesCpp::PhysDesCpp cxxopts)
