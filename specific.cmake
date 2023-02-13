cpmaddpackage(
  NAME
  fmt
  GIT_TAG
  7.1.3
  GITHUB_REPOSITORY
  fmtlib/fmt
  OPTIONS
  "FMT_INSTALL YES" # create an installable target
)

cpmaddpackage("gh:microsoft/GSL@3.1.0")
set(SPECIFIC_LIBS fmt::fmt GSL)

# cpmaddpackage(
#   NAME GSL
#   GITHUB_REPOSITORY "microsoft/GSL"
#   GIT_TAG "v4.0.0"
#   GIT_SHALLOW ON
# )

# include(FetchContent)
#
# FetchContent_Declare(GSL
#     GIT_REPOSITORY "https://github.com/microsoft/GSL"
#     GIT_TAG "v4.0.0"
#     GIT_SHALLOW ON
# )

# FetchContent_MakeAvailable(GSL)

# find_package(Microsoft.GSL CONFIG REQUIRED)

# set(SPECIFIC_LIBS fmt::fmt Microsoft.GSL::GSL)
