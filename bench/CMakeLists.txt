cmake_minimum_required(VERSION 3.14...3.22)

project(RectiBenchmarks LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)
include(../cmake/specific.cmake)

# nanobench is a header-only library for microbenchmarking
CPMAddPackage(
  NAME nanobench
  GITHUB_REPOSITORY martinus/nanobench
  GIT_TAG v4.3.11
)

CPMAddPackage(NAME Recti SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create binaries ----

file(GLOB_RECURSE ALL_BENCH_CPP *.cpp)

enable_testing()

foreach(ONE_BENCH_CPP ${ALL_BENCH_CPP})

  get_filename_component(ONE_BENCH_EXEC ${ONE_BENCH_CPP} NAME_WE)

  # Avoid name collision
  set(TARGET_NAME Bench_${ONE_BENCH_EXEC})

  add_executable(${TARGET_NAME} ${ONE_BENCH_CPP})
  set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_BENCH_EXEC})
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)

  target_link_libraries(${TARGET_NAME} nanobench Recti::Recti ${SPECIFIC_LIBS})

  add_test(NAME ${ONE_BENCH_EXEC} COMMAND ${TARGET_NAME})
endforeach()
