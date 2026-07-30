cmake_minimum_required(VERSION 3.14...3.22)

project(RectiTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

if(TEST_INSTALLED_VERSION)
  # All dependencies are installed — use find_package to avoid
  # CPM re-fetching fmt/spdlog transitively, causing ALIAS conflicts
  find_package(fmt REQUIRED)
  find_package(spdlog REQUIRED)
  find_package(Microsoft.GSL CONFIG REQUIRED)
  find_package(LdsGen REQUIRED)
  set(SPECIFIC_LIBS LdsGen::LdsGen fmt::fmt Microsoft.GSL::GSL spdlog::spdlog)
  find_package(Recti REQUIRED)
else()
  include(../cmake/specific.cmake)

  if(NOT TARGET Recti)
    CPMAddPackage(NAME Recti SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
  endif()
endif()

# Try system-installed doctest first (Ubuntu: doctest-dev, macOS: brew install doctest, Termux: doctest)
find_package(doctest CONFIG QUIET)

if(NOT doctest_FOUND)
  CPMAddPackage("gh:doctest/doctest@2.5.2")
endif()
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

# Add RapidCheck for property-based testing CPMAddPackage( NAME rapidcheck GITHUB_REPOSITORY
# emil-e/rapidcheck GIT_TAG master OPTIONS "RC_BUILD_TESTS OFF" "RC_BUILD_EXAMPLES OFF" "RC_INSTALL
# OFF" )

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} doctest::doctest Recti::Recti ${SPECIFIC_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# Add RapidCheck support if available if(rapidcheck_ADDED)
# target_compile_definitions(${PROJECT_NAME} PRIVATE RAPIDCHECK_H) if(TARGET rapidcheck::rapidcheck)
# target_link_libraries(${PROJECT_NAME} rapidcheck::rapidcheck) else()
# target_include_directories(${PROJECT_NAME} PRIVATE ${rapidcheck_SOURCE_DIR}/include) if(TARGET
# rapidcheck) target_link_libraries(${PROJECT_NAME} rapidcheck) endif() endif() endif()

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(Recti PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(Recti PUBLIC /utf-8 /W4 /WX)
    # target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add RectiTests ----

enable_testing()

# Add a single test that runs all tests (both doctest and RapidCheck) Note: doctest auto-discovery
# can conflict with other testing frameworks like RapidCheck
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(Recti PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(Recti PUBLIC -fprofile-arcs -ftest-coverage)
endif()
