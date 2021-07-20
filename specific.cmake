CPMAddPackage(
  NAME fmt
  GIT_TAG 7.1.3
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

CPMAddPackage("gh:microsoft/GSL@3.1.0")

# CPMAddPackage( NAME Boost VERSION 1.76.0 URL
# https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz DOWNLOAD_ONLY
# True ) if(Boost_ADDED) # Define the header-only Boost target add_library(Boost::boost INTERFACE
# IMPORTED GLOBAL) target_include_directories(Boost::boost SYSTEM INTERFACE ${Boost_SOURCE_DIR})

# # Disable autolink target_compile_definitions(Boost::boost INTERFACE BOOST_ALL_NO_LIB=1) endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
  message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
  # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
  target_include_directories(Boost::boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
  # Disable autolink
  target_compile_definitions(Boost::boost INTERFACE BOOST_ALL_NO_LIB=1)
endif()

set(SPECIFIC_LIBS fmt::fmt Boost::boost GSL)
