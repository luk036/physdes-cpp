cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(RectiStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 2.2.1
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)
CPMAddPackage(NAME Recti SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
include(../specific.cmake)

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(RectiStandalone ${sources})

set_target_properties(RectiStandalone PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "Recti")

target_link_libraries(RectiStandalone Recti::Recti cxxopts ${SPECIFIC_LIBS})

# target_include_directories( RectiStandalone PUBLIC
# $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
# $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}> PRIVATE ${BOOST_INCLUDE_DIRS} )
