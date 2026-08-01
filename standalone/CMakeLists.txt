cmake_minimum_required(VERSION 3.14...3.22)

project(RectiStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

# If SPECIFIC_LIBS is already set, deps were handled by the parent (e.g. test_installed). Otherwise,
# pull them via specific.cmake.
if(NOT SPECIFIC_LIBS)
  include(../cmake/specific.cmake)
endif()

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.2.1
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

if(NOT TARGET Recti::Recti)
  CPMAddPackage(NAME Recti SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "Recti")

target_link_libraries(${PROJECT_NAME} Recti::Recti cxxopts::cxxopts ${SPECIFIC_LIBS})
