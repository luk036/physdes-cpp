cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(RectiTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:onqtam/doctest#2.4.5")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.0")

include(../specific.cmake)

if(TEST_INSTALLED_VERSION)
  find_package(Recti REQUIRED)
else()
  CPMAddPackage(NAME Recti SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(RectiTests ${sources})
target_link_libraries(RectiTests doctest::doctest Recti::Recti GSL Boost::boost)
set_target_properties(RectiTests PROPERTIES CXX_STANDARD 17)

# target_include_directories( RectiTests PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
# $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}> PRIVATE  ${BOOST_INCLUDE_DIRS} )

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(Recti PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(Recti PUBLIC /W4 /WX)
    target_compile_definitions(RectiTests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add RectiTests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME rectiTests COMMAND RectiTests)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(RectiTests)

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(Recti PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(Recti PUBLIC -fprofile-arcs -ftest-coverage)
endif()
