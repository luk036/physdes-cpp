# Benchmark Comparison: Polygon vs. RPolygon

This report details the process and results of benchmarking the `Polygon` and `RPolygon` classes within the `physdes-cpp` project.

## 1. Introduction

The primary goal of this task was to compare the performance of the `Polygon` and `RPolygon` classes, specifically for area calculation. This was achieved by introducing a new benchmark suite using the Google Benchmark library. The benchmark was integrated into the existing `xmake` build system and a new `CMake` configuration was also added.

## 2. Implementation

### 2.1. Build System Integration

The benchmark was integrated into both `xmake` and `CMake` build systems.

#### 2.1.1. xmake

- **Dependency**: The `benchmark` package was added as a requirement in `xmake.lua`.
- **Target**: A new target `benchmark_recti` was created to build and run the benchmarks.

#### 2.1.2. CMake

- **Dependency**: Google Benchmark was added using `CPM.cmake` in `all/CMakeLists.txt`.
- **Configuration**:
    - A new `bench/CMakeLists.txt` file was created to define the `benchmark_recti` executable.
    - The C++ standard was set to C++20 in `all/CMakeLists.txt` to resolve compilation issues.
    - The `standalone` project was temporarily disabled to resolve a build error.

### 2.2. Benchmark Implementation

A new benchmark file `bench/bench_main.cpp` was created with two test cases:
- `BM_Polygon_Area`: Measures the time to calculate the area of a `Polygon` object representing a square.
- `BM_RPolygon_Area`: Measures the time to calculate the area of an `RPolygon` object representing the same square.

An important correction was made to the `RPolygon` benchmark. It was noted that `RPolygon` has a specialized internal representation that only stores the even-indexed vertices of a rectilinear polygon. The benchmark was adjusted to initialize the `RPolygon` with only two opposite corner points of the square, which is the correct way to represent a rectangle with `RPolygon`.

## 3. Results

The benchmarks were run on a Windows machine with an Intel CPU @ 2.995 GHz. The results for calculating the area of a square are as follows:

### 3.1. xmake Build Results

```
-----------------------------------------------------------
Benchmark                 Time             CPU   Iterations
-----------------------------------------------------------
BM_Polygon_Area        5.89 ns         5.84 ns    144516129
BM_RPolygon_Area       4.11 ns         3.94 ns    154482759
```

### 3.2. CMake Build Results

```
-----------------------------------------------------------
Benchmark                 Time             CPU   Iterations
-----------------------------------------------------------
BM_Polygon_Area        6.30 ns         6.10 ns     89600000
BM_RPolygon_Area       4.48 ns         4.30 ns    160000000
```

## 4. Conclusion

The benchmark results from both `xmake` and `CMake` builds consistently show that the `RPolygon::signed_area()` function is significantly faster than `Polygon::signed_area_x2()`. This performance difference can be attributed to the specialized and more compact representation of `RPolygon`, which simplifies the area calculation. The results validate the efficiency of the `RPolygon` implementation for rectilinear shapes.
