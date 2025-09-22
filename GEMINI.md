# Project: physdes-cpp

## Project Overview

This is a C++ project for physical design, named "Recti". It appears to be a header-only library designed with modern C++ and CMake practices. The project is structured to be a template, allowing for easy adaptation to new projects.

**Key Technologies:**

*   **Language:** C++
*   **Build System:** CMake
*   **Dependency Management:** CPM.cmake
*   **Testing:** CTest (with doctest)
*   **CI/CD:** GitHub Actions
*   **Code Coverage:** Codecov
*   **Code Formatting:** clang-format, cmake-format
*   **Documentation:** Doxygen

**Architecture:**

The project follows a clean structure, separating the library code from executables and tests.

*   `include/recti/`: Contains the header-only library files.
*   `source/`: Contains C++ source files (if any).
*   `test/`: Contains the test suite for the library.
*   `standalone/`: Contains a standalone executable demonstrating the library usage.
*   `all/`: A convenience directory to build all targets (library, tests, standalone) at once.
*   `cmake/`: Contains CMake helper scripts.
*   `.github/workflows/`: Contains CI/CD pipeline configurations for different platforms.

## Building and Running

The project can be built and tested using CMake and CTest.

### Build Everything

This is the recommended approach for development as it builds all targets (library, tests, standalone).

```bash
cmake -S all -B build
cmake --build build
```

### Running Tests

After building everything, you can run the tests:

```bash
./build/test/RectiTests
```

Alternatively, using CTest:

```bash
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build --target test
```

### Running the Standalone Executable

```bash
./build/standalone/Recti --help
```

### Building Documentation

To build the Doxygen documentation:

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# Open the documentation
open build/doc/doxygen/html/index.html
```

## Development Conventions

*   **Coding Style:** The project uses `.clang-format` for C++ code and `.cmake-format` for CMake files to enforce a consistent coding style. Use the `format` and `fix-format` targets to check and apply the style.
    ```bash
    # Check formatting
    cmake --build build --target format
    # Fix formatting
    cmake --build build --target fix-format
    ```
*   **Dependencies:** Dependencies are managed via `CPM.cmake`. Add new dependencies in the `CMakeLists.txt` file.
*   **Testing:** Tests are written using the doctest framework and located in the `test/source` directory. New tests should be added there.
*   **Continuous Integration:** The project uses GitHub Actions for continuous integration. The workflows are defined in the `.github/workflows` directory. Any changes should pass the CI checks.
