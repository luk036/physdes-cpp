CPMAddPackage(
  NAME fmt
  GIT_TAG 9.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

CPMAddPackage("gh:microsoft/GSL@4.0.0")

CPMAddPackage(
  NAME LdsGen
  GIT_TAG 1.0.1
  GITHUB_REPOSITORY luk036/lds-gen-cpp
  OPTIONS "INSTALL_ONLY YES" # create an installable target
)

set(SPECIFIC_LIBS LdsGen::LdsGen fmt::fmt Microsoft.GSL::GSL)

# cpmaddpackage( NAME GSL GITHUB_REPOSITORY "microsoft/GSL" GIT_TAG "v4.0.0" GIT_SHALLOW ON )

# include(FetchContent)
#
# FetchContent_Declare(GSL GIT_REPOSITORY "https://github.com/microsoft/GSL" GIT_TAG "v4.0.0"
# GIT_SHALLOW ON )

# FetchContent_MakeAvailable(GSL)

# find_package(Microsoft.GSL CONFIG REQUIRED)

# set(SPECIFIC_LIBS fmt::fmt Microsoft.GSL::GSL)
