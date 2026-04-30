CPMAddPackage(
  NAME fmt
  GIT_TAG 12.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" "FMT_USE_INLINE_VTABLE OFF" "FMT_DISABLE_COMPILE_STRING" # create an installable target
)

CPMAddPackage(
  NAME GSL
  GITHUB_REPOSITORY "microsoft/GSL"
  GIT_TAG "v4.0.0"
  GIT_SHALLOW ON
  OPTIONS "GSL_INSTALL YES"
)

CPMAddPackage(
  NAME LdsGen
  GIT_TAG 1.1
  GITHUB_REPOSITORY luk036/lds-gen-cpp
  OPTIONS "INSTALL_ONLY YES" # create an installable target
)

CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.17.0
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_RUNTIME_CHECKS OFF" "FMT_DEPRECATED_EXTERNAL_ABI" "SPDLOG_FMT_EXTERNAL" # create an installable target
)

set(SPECIFIC_LIBS LdsGen::LdsGen fmt::fmt Microsoft.GSL::GSL spdlog::spdlog)

# cpmaddpackage( NAME GSL GITHUB_REPOSITORY "microsoft/GSL" GIT_TAG "v4.0.0" GIT_SHALLOW ON )

# include(FetchContent)
#
# FetchContent_Declare(GSL GIT_REPOSITORY "https://github.com/microsoft/GSL" GIT_TAG "v4.0.0"
# GIT_SHALLOW ON )

# FetchContent_MakeAvailable(GSL)

# find_package(Microsoft.GSL CONFIG REQUIRED)

# set(SPECIFIC_LIBS fmt::fmt Microsoft.GSL::GSL)
