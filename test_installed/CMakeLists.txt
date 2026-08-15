cmake_minimum_required(VERSION 3.14)
project(RectiInstalledTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies first - the installed config references fmt and Lds.
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Lds REQUIRED)
find_package(Recti REQUIRED)

add_executable(recti_installed_test main.cpp)
target_link_libraries(recti_installed_test PRIVATE Recti::Recti Threads::Threads)

enable_testing()
add_test(NAME recti_installed_test COMMAND recti_installed_test)
