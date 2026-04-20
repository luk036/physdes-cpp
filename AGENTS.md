# AGENTS.md - Agentic Development Guide

## Project Overview

- **Name**: Recti (Physical Design Library)
- **Type**: Modern C++ library with CMake build system
- **C++ Standard**: C++20
- **Test Framework**: doctest + RapidCheck (property-based)

## Directory Structure

```
physdes-cpp/
├── include/recti/      # Public headers (*.h, *.hpp)
├── source/             # Implementation files (*.cpp)
├── test/source/        # Unit tests (*.cpp)
├── standalone/         # Standalone executable
├── cmake/              # CMake modules
└── .github/workflows/  # CI pipelines
```

## Build Commands

### Full Build (All Targets)

```bash
cmake -S all -B build
cmake --build build
```

### Run Test Suite

```bash
cmake -S test -B build/test
cmake --build build/test

# Option 1: Via CTest
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# Option 2: Direct executable
./build/test/RectiTests
# Run specific test:
./build/test/RectiTests -tc="Vector2"  # Run tests matching "Vector2"
```

### Run Single Test Case

```bash
./build/test/RectiTests -tc="test_case_name"
./build/test/RectiTests -ts="test_suite_name"
./build/test/RectiTests -tc="*" -ts="test_suite_name"  # All cases in suite
```

### Standalone Executable

```bash
cmake -S standalone -B build/standalone
cmake --build build/standalone
./build/standalone/Recti --help
```

### Code Formatting

```bash
# View changes (dry-run)
cmake --build build/test --target format

# Apply fixes
cmake --build build/test --target fix-format

# Format dependencies (pip):
pip install clang-format==18.1.2 cmake_format==0.6.13 pyyaml
```

### Code Coverage

```bash
cmake -S test -B build/test -DENABLE_TEST_COVERAGE=1
cmake --build build/test
./build/test/RectiTests
```

### Sanitizers

```bash
cmake -S test -B build/test -DUSE_SANITIZER=Address
cmake -S test -B build/test -DUSE_SANITIZER='Address;Undefined'
```

### Static Analyzers

```bash
cmake -S test -B build/test -DUSE_STATIC_ANALYZER=clang-tidy
cmake -S test -B build/test -DUSE_STATIC_ANALYZER='clang-tidy;cppcheck'
```

## Code Style Guidelines

### Formatting (`.clang-format`)

- **Base Style**: Google
- **Column Limit**: 100
- **Indent Width**: 4
- **Brace Style**: Attach
- **Namespace Indentation**: All
- **Always Break**: Before binary operators, ternary operators
- **Include Sorting**: Regroup (alphabetically within groups)

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Files | snake_case | `vector2.hpp`, `global_router.cpp` |
| Classes | PascalCase | `class Vector2`, `class Rpolygon` |
| Functions/Methods | camelCase | `getX()`, `computeDistance()` |
| Member Variables | underscore prefix | `_x`, `_y` |
| Constants | UPPER_SNAKE_CASE | `MAX_BUFFER_SIZE` |
| Namespaces | lowercase | `recti`, `detail` |

### Include Order

1. Corresponding header (if implementation file)
2. Local headers (`#include "..."`)
3. C++ standard library (`#include <...>`)
4. External libraries (from CPM)

```cpp
#include "vector2.hpp"           // Local

#include <vector>               // C++ STL
#include <algorithm>             // C++ STL

#include <doctest/doctest.h>   // External
#include <fmt/core.h>           // External (fmt)
```

### Compiler Flags

- **GCC/Clang**: `-Wall -Wpedantic -Wextra -Werror -Wno-deprecated-declarations`
- **MSVC**: `/W4 /WX /wd4819 /wd4996`

### Error Handling

- Prefer `noexcept` specifiers on functions that cannot throw
- Use `std::runtime_error` for recoverable runtime errors
- Avoid exceptions in headers where possible - return `std::optional<T>` instead
- Use assertions (`assert()`) for invariants that MUST hold

### Type Annotations

- Use trailing return type syntax where cleaner: `auto foo() const -> int`
- Prefer `const` correctness - mark members `const` when possible
- Use `constexpr` for compile-time evaluation
- Avoid raw pointers - prefer `std::unique_ptr`, `std::shared_ptr`

### Testing Patterns (doctest)

```cpp
#include <doctest/doctest.h>

TEST_CASE("Feature Description") {
    SUBCASE("Sub-case") {
        CHECK_EQ(expected, actual);
        CHECK_THROWS_AS(expression, exception_type);
    }
}
```

### Key Dependencies (CPM.cmake)

- `doctest@2.4.11` - Testing framework
- `rapidcheck` - Property-based testing (master branch)
- `PackageProject.cmake@1.8.0` - Installable targets
- `Format.cmake@1.7.3` - Code formatting

## Relevant Files

- Main CMakeLists: `CMakeLists.txt`
- Test CMakeLists: `test/CMakeLists.txt`
- Main header: `include/recti/recti.hpp`
- Sample header: `include/recti/vector2.hpp`
- Sample test: `test/source/test_vector2.cpp`