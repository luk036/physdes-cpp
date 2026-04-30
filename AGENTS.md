# AGENTS.md - Agentic Development Guide

## Project Overview

- **Name**: Recti (Physical Design Library)
- **Type**: Modern C++ library with CMake build system
- **C++ Standard**: C++20
- **Test Framework**: doctest + RapidCheck (property-based testing)

## Directory Structure

```
physdes-cpp/
├── include/recti/      # Public headers (*.h, *.hpp)
├── source/             # Implementation files (*.cpp)
├── test/source/        # Unit tests (*.cpp)
├── standalone/         # Standalone executable
├── cmake/              # CMake modules
├── all/                # Combined build (all targets)
├── bench/              # Benchmarks
└── .github/workflows/  # CI pipelines
```

## Build Commands

### Full Build (All Targets)

```bash
cmake -S all -B build
cmake --build build
```

### Test Suite

```bash
cmake -S test -B build/test
cmake --build build/test

# Via CTest
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# Direct executable
./build/test/RectiTests
```

### Running Single Tests (doctest filters)

```bash
# Run specific test case (exact match)
./build/test/RectiTests -tc="Vector2.DefaultConstructor"

# Run tests matching pattern (wildcard)
./build/test/RectiTests -tc="Vector2*"       # All Vector2 tests
./build/test/RectiTests -tc="*Constructor*"   # Any constructor tests

# Run test suite
./build/test/RectiTests -ts="Vector2"         # All cases in Vector2 suite
./build/test/RectiTests -tc="*" -ts="Vector2" # Explicit suite+wildcard

# List available tests without running
./build/test/RectiTests -ltc                    # List test cases
./build/test/RectiTests -lts                   # List test suites
```

### Standalone Executable

```bash
cmake -S standalone -B build/standalone
cmake --build build/standalone
./build/standalone/Recti --help
```

### Code Formatting (C++ and CMake)

```bash
# View changes (dry-run)
cmake --build build/test --target format

# Apply fixes
cmake --build build/test --target fix-format

# Dependencies (pip)
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

| Setting | Value |
|---------|-------|
| Base Style | Google |
| Column Limit | 100 |
| Indent Width | 4 |
| Brace Style | Attach |
| Namespace Indentation | All |
| Break Before | Binary operators, ternary operators |
| Include Sorting | Regroup (alphabetically within groups) |

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Files | snake_case | `vector2.hpp`, `global_router.cpp` |
| Classes | PascalCase | `class Vector2`, `class RPolygon` |
| Functions/Methods | camelCase | `getX()`, `computeDistance()` |
| Member Variables | underscore prefix | `_x`, `_y` |
| Constants | UPPER_SNAKE_CASE | `MAX_BUFFER_SIZE` |
| Namespaces | lowercase | `recti`, `detail` |

### Include Order

1. Corresponding header (implementation file)
2. Local headers (`#include "..."`)
3. C++ standard library (`#include <...>`)
4. External libraries (CPM)

```cpp
#include "vector2.hpp"           // Local

#include <vector>               // C++ STL
#include <algorithm>             // C++ STL

#include <doctest/doctest.h>   // External (CPM)
#include <fmt/core.h>           // External (CPM)
```

### Compiler Flags

- **GCC/Clang**: `-Wall -Wpedantic -Wextra -Werror -Wno-deprecated-declarations`
- **MSVC**: `/W4 /WX /wd4459 /wd4819 /wd4996`

### Error Handling

- Use `noexcept` on functions that cannot throw
- Prefer `std::optional<T>` in headers instead of exceptions
- Use `std::runtime_error` for recoverable runtime errors
- Use `assert()` for invariants that MUST hold (debug builds)

```cpp
// Good: Return optional for failure cases in headers
auto divide(int a, int b) const -> std::optional<int> {
    if (b == 0) return std::nullopt;
    return a / b;
}

// Good: Use assertions for invariants
auto Vector2::normalize() -> void {
    assert(_magnitude > 0 && "Cannot normalize zero vector");
    // ...
}
```

### Type Annotations

- Prefer trailing return type: `auto foo() const -> int`
- Mark members `const` when possible
- Use `constexpr` for compile-time evaluation
- Prefer `std::unique_ptr` / `std::shared_ptr` over raw pointers

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

### RapidCheck (Property-Based Testing)

```cpp
#ifdef RAPIDCHECK_H
#include <rapidcheck.h>

TEST_CASE("Vector2 operations") {
    rc::prop("addition is commutative", []() {
        Vector2 a = *rc::gen::inRange(-100, 100);
        Vector2 b = *rc::gen::inRange(-100, 100);
        RC_ASSERT(a + b == b + a);
    });
}
#endif
```

## Key Dependencies (CPM.cmake)

| Package | Version | Purpose |
|---------|---------|---------|
| doctest | 2.4.11 | Testing framework |
| rapidcheck | master | Property-based testing |
| PackageProject.cmake | 1.8.0 | Installable targets |
| Format.cmake | 1.7.3 | Code formatting |

## Relevant Files

- Main CMakeLists: `CMakeLists.txt`
- Test CMakeLists: `test/CMakeLists.txt`
- Main header: `include/recti/recti.hpp`
- Sample header: `include/recti/vector2.hpp`
- Sample test: `test/source/test_vector2.cpp`
- .clang-format: `.clang-format`