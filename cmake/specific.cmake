# Try system-installed fmt first (Ubuntu: libfmt-dev, macOS: brew install fmt, Termux: fmt)
find_package(fmt CONFIG QUIET)

if(fmt_FOUND)
  message(STATUS "Found system fmt: ${fmt_DIR}")
  # Tell CPM that fmt is already handled (CPM checks CPM_PACKAGES list). Write the CACHE variable
  # directly: list(APPEND ...) creates a normal-variable shadow that does not propagate into
  # FetchContent subdirectory scopes.
  if(NOT fmt IN_LIST CPM_PACKAGES)
    set(CPM_PACKAGES
        "${CPM_PACKAGES};fmt"
        CACHE INTERNAL "" FORCE
    )
  endif()
else()
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" "FMT_USE_INLINE_VTABLE OFF" "FMT_DISABLE_COMPILE_STRING" # create an
            # installable target
  )
endif()

CPMAddPackage(
  NAME GSL
  GITHUB_REPOSITORY "microsoft/GSL"
  GIT_TAG "v4.0.0"
  GIT_SHALLOW ON
  OPTIONS "GSL_INSTALL YES"
)

# Try system-installed spdlog first (Ubuntu: libspdlog-dev, macOS: brew install spdlog, Termux:
# spdlog) Note: this must come BEFORE LdsGen is added: LdsGen's own dependency chain CPM-adds
# spdlog, so registering spdlog here first lets CPM skip the duplicate (and its bundled-fmt build).
find_package(spdlog CONFIG QUIET)

if(spdlog_FOUND)
  message(STATUS "Found system spdlog: ${spdlog_DIR}")
  # Tell CPM that spdlog is already handled (write CACHE directly, see fmt above)
  if(NOT spdlog IN_LIST CPM_PACKAGES)
    set(CPM_PACKAGES
        "${CPM_PACKAGES};spdlog"
        CACHE INTERNAL "" FORCE
    )
  endif()
else()
  # When fmt is from system, tell spdlog to use it externally to avoid its bundled fmt conflicting
  # with the installed fmt::fmt targets.
  if(fmt_FOUND)
    set(SPDLOG_FMT_EXTERNAL YES)
  endif()
  CPMAddPackage(
    NAME spdlog
    GIT_TAG v1.17.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_RUNTIME_CHECKS OFF" "FMT_DEPRECATED_EXTERNAL_ABI"
            "SPDLOG_FMT_EXTERNAL ${SPDLOG_FMT_EXTERNAL}" # create an installable target
  )
endif()

CPMAddPackage(
  NAME LdsGen
  GIT_TAG v1.2.4
  GITHUB_REPOSITORY luk036/lds-gen-cpp
  OPTIONS "INSTALL_ONLY YES" # create an installable target
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
