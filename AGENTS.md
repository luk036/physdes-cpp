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
cmake -B build
cmake --build build
```

### Test Suite

```bash
cmake -B build
cmake --build build

# Via CTest
ctest --test-dir build --output-on-failure

# Direct executable
./build/RectiTests
```

### Running Single Tests (doctest filters)

```bash
# Run specific test case (exact match)
./build/RectiTests -tc="Vector2.DefaultConstructor"

# Run tests matching pattern (wildcard)
./build/RectiTests -tc="Vector2*"       # All Vector2 tests
./build/RectiTests -tc="*Constructor*"   # Any constructor tests

# Run test suite
./build/RectiTests -ts="Vector2"         # All cases in Vector2 suite
./build/RectiTests -tc="*" -ts="Vector2" # Explicit suite+wildcard

# List available tests without running
./build/RectiTests -ltc                    # List test cases
./build/RectiTests -lts                   # List test suites
```

### Standalone Executable

```bash
cmake -B build
cmake --build build
./build/Recti --help
```

### Code Formatting (C++ and CMake)

```bash
# View changes (dry-run)
cmake --build build --target format

# Apply fixes
cmake --build build --target fix-format

# Dependencies (pip)
pip install clang-format==18.1.2 cmake_format==0.6.13 pyyaml
```

### Code Coverage

```bash
cmake -B build -DENABLE_TEST_COVERAGE=1
cmake --build build
./build/RectiTests
```

### Sanitizers

```bash
cmake -B build -DUSE_SANITIZER=Address
cmake -B build -DUSE_SANITIZER='Address;Undefined'
```

### Static Analyzers
```bash
# clang-tidy over the public headers (uses the checks in .clang-tidy)
cmake -S . -B build -DRECTI_ENABLE_CLANG_TIDY=ON
cmake --build build --target clang-tidy
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
- **MSVC**: `/utf-8 /W4 /WX`

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

## RPolygon Representation

The canonical point-sequence representation of a rectilinear polygon is the
**even-index representation**, not a full vertex list.

The `RPolygon<T>` class is a separate explicit representation. It stores
`origin + edge vectors`, its point-set constructor treats the input as a full
vertex list, and `vertices()` returns the full boundary.

### Canonical encoding

Start from a simple rectilinear polygon with `n` corner vertices (remove
collinear vertices first). Rotate the full vertex list so that its first edge
is horizontal. Store every other vertex:

```text
S[i] = full[(start + 2*i) % n],   i = 0 .. n/2 - 1
```

The omitted vertex between two stored vertices is recovered by moving
**x first, then y**:

```text
S[i]   = (x0, y0)
S[i+1] = (x1, y1)
implicit vertex = (x1, y0)
```

Therefore the full boundary is:

```text
S0, (S1.x, S0.y), S1, (S2.x, S1.y), S2, ...
```

Example:

```text
stored:  (0,0) (3,3) (2,1) (1,3)
full:    (0,0) (3,0) (3,3) (2,3),
         (2,1) (1,1) (1,3) (0,3)
```

Do **not** use the y-first form `(x0, y1)`; that reconstructs a different
polygon.

### Invariants

- The list has at least **2** stored points.
- Consecutive stored points differ in **both** `x` and `y`. If they share an
  axis, the list is not a canonical even-index representation.
- The stored order follows the boundary order; do not reverse it.
- Start on a vertex whose outgoing edge is horizontal. Starting on a vertical
  edge makes the x-first rule reconstruct a different polygon.
- A rectangle is stored as **2** opposite corners. A 2-point representation
  whose points share `x` or `y` is invalid, not a thin rectangle.
- A monotone/L-shaped (orthogonally convex) polygon is stored as **3** points.
- More stored points describe a larger orthogonally convex polygon.
- The full boundary is reconstructed on demand; it is not the stored form.

### Representation forms

- 2 stored points: rectangle, stored as opposite corners.
- 3 stored points: L-shaped / orthogonally convex polygon.
- More stored points: larger orthogonally convex polygon.
- A 2-point or 3-point representation is valid, not degenerate.

### Deriving test fixtures

Build fixtures from the canonical full vertex list for every RPolygon test. Do
not hand-write stored points or pass a raw full boundary.

```cpp
const std::vector<Point<int, int>> full = {
    {0, 0}, {3, 0}, {3, 3}, {2, 3},
    {2, 1}, {1, 1}, {1, 3}, {0, 3}};

// full[0] -> full[1] is horizontal, so start = 0
std::vector<Point<int, int>> stored;
for (std::size_t i = 0; i < full.size(); i += 2) {
    stored.push_back(full[i]);
}
// stored == {(0,0), (3,3), (2,1), (1,3)}
```

If the full list starts with a vertical edge, rotate it until the first edge is
horizontal before taking every other vertex. `create_test_rpolygon` returns a
full boundary; convert its output with the same procedure before using it as an
even-index fixture.

## Key Dependencies (CPM.cmake)

| Package | Version | Purpose |
|---------|---------|---------|
| doctest | 2.5.2 | Testing framework |
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
