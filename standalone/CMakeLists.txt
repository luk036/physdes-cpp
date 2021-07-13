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

# CPMAddPackage(
#   NAME Boost
#   VERSION 1.76.0
#   URL https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz
#   DOWNLOAD_ONLY True 
# )
# if(Boost_ADDED)
#   # Define the header-only Boost target
#   add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
#   target_include_directories(Boost::boost SYSTEM INTERFACE ${Boost_SOURCE_DIR})

#   # Disable autolink
#   target_compile_definitions(Boost::boost INTERFACE BOOST_ALL_NO_LIB=1)
# endif()


find_package(Boost REQUIRED)
if(Boost_FOUND)
  message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
  # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
  target_include_directories(Boost::boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
  # Disable autolink
  target_compile_definitions(Boost::boost INTERFACE BOOST_ALL_NO_LIB=1)
endif()


# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(PhysDesCppStandalone ${sources})

set_target_properties(PhysDesCppStandalone PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "PhysDesCpp")

target_link_libraries(PhysDesCppStandalone PhysDesCpp::PhysDesCpp cxxopts GSL Boost::boost)

# target_include_directories(
#   PhysDesCppStandalone PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#                     $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
#                     PRIVATE ${BOOST_INCLUDE_DIRS}
# )
