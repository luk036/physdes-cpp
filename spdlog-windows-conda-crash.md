# Debug-Build Crash: `std::string` ABI Mismatch with Conda fmt/spdlog DLLs on Windows

**Date**: 2026-08-15
**Project**: physdes-cpp (Recti) — `D:\github\cpp\physdes-cpp`
**Reporter**: Sisyphus (AI agent), on request of the maintainer
**Severity**: High (Debug builds unusable; all tests crash/skip)
**Status**: Fixed (CMakeLists.txt change) and verified

---

## 1. Executive Summary

The CMake **Debug** build (MSVC) of physdes-cpp crashed in 7 test cases (SIGSEGV / "Terminate handler called"), causing doctest to skip 144 of 154 test cases. The root cause was **not** a bug in the application code (DME algorithm / global router) but an **ABI mismatch of `std::string`**:

- CMake resolved `fmt`/`spdlog` to **conda's Release-only shared libraries** (`fmt.dll`, `spdlog.dll`).
- The conda DLLs are compiled with `_ITERATOR_DEBUG_LEVEL=0` (Release), while the project's Debug build uses `_ITERATOR_DEBUG_LEVEL=2`.
- MSVC `std::string` has a **different memory layout** between these two levels (the Debug layout carries a `_Container_proxy*`).
- Every `std::string` returned from a conda DLL (e.g. the result of `fmt::format(...)`) was therefore misinterpreted by the Debug caller, yielding a corrupt string (`size=15, data=nullptr` for a 2-char string). Copying it crashed.

The fix: on MSVC, do **not** use prebuilt system fmt/spdlog — build them from source via CPM (the same path CI already takes, which is why CI was green). After the fix, **154/154 tests pass in Debug and Release** (766/766 assertions).

---

## 2. Original Symptoms

### 2.1 Environment

- OS: Windows, PowerShell 7
- Toolchain: MSVC (Visual Studio 18 / MSVC 14.51), C++20
- Build systems: CMake (VS multi-config generator, Debug default) and xmake (Release default)
- Dependencies: system (conda) fmt 12.1.0 + spdlog 1.17.0 found by `find_package`, plus CPM-managed Lds / doctest / cxxopts

### 2.2 Failure mode (CMake Debug)

Running the full suite produced:

```
[doctest] test cases: 10 | 9 passed | 1 failed | 144 skipped
[doctest] assertions: 42 | 42 passed | 0 failed |
[doctest] Status: FAILURE!
```

A single crash aborted the run and everything after it was reported as "skipped".

### 2.3 Crashing test cases (7 total)

| Test file | Test case | Failure |
|---|---|---|
| `test/source/test_clk_tree_vis.cpp` (line 252) | "Visualize DME Generated Tree" | **SIGSEGV** |
| `test/source/test_clk_tree_vis_random.cpp` (line 29) | "Random sink stress test" | **SIGSEGV** |
| `test/source/test_global_router.cpp` (line 34) | "Test routing algorithms" (SUBCASE `route_with_steiners`) | **Terminate handler called** |
| `test/source/test_global_router3d.cpp` (line 43) | "Test routing algorithms in 3D" | **Terminate handler called** |
| `test/source/test_global_router3d_with_keepouts.cpp` (line 47) | "Test routing algorithms in 3D with keepouts" | **Terminate handler called** |
| `test/source/test_global_router_with_keepouts.cpp` (line 43) | "Test routing algorithms with keepouts" | **Terminate handler called** |
| `test/source/test_global_router_stress.cpp` | "GlobalRouter Stress Tests" | **Terminate handler called** |

All crashes occurred with **0 assertions executed** — the failure happened inside the DME/router code before any `CHECK`, and doctest's exception/signal handler then skipped the remaining test cases.

### 2.4 Key observation: Debug-only

- **xmake Release** build: `154/154 passed` (766 assertions).
- **CMake Release** build: `154/154 passed`.
- **CMake Debug** build: crashed as above.

This Debug-only signature pointed away from a plain logic bug and toward Debug instrumentation, undefined behavior, or a Debug/Release configuration difference.

---

## 3. Investigation Trail

### 3.1 Ruling out the Lds swap (baseline comparison)

The crash investigation started right after a large dependency swap (LdsGen → Lds). To prove the crash was **not introduced** by that change, the unmodified baseline was rebuilt via `git stash` cycles:

- Baseline (original LdsGen code, Debug): same crashes, same files, same line numbers.
- Post-swap (Lds code, Debug): identical crashes.

**Conclusion**: pre-existing failure, unrelated to the Lds migration.

### 3.2 Ruling out CI ("Why is `gh run list` green?")

CI (`windows.yml`, `ubuntu.yml`) configures **`-DCMAKE_BUILD_TYPE=Release`**. Two reasons CI never saw the crash:

1. CI builds **Release**, and the crash is Debug-only.
2. `windows-latest` has **no conda** → `find_package(fmt)` fails → CMake falls back to **CPM source builds** of fmt/spdlog, which match the project's ABI in any configuration.

### 3.3 Getting a real stack trace (AddressSanitizer)

`cdb.exe` was not installed, so an **MSVC AddressSanitizer** build was used:

```powershell
cmake -B build-asan -DRECTI_BUILD_TESTS=ON -DCMAKE_CXX_FLAGS="/fsanitize=address /Zi /EHsc"
cmake --build build-asan --target RectiTests
```

Notes:
- `clang_rt.asan_dynamic-x86_64.dll` had to be added to `PATH` (lives in `VC\Tools\MSVC\<ver>\bin\Hostx64\x64`).
- The project's `/WX` (warnings-as-errors) required `/EHsc` in the flags.

ASan produced the decisive stack trace:

```
#1 memcpy
#2 std::_Char_traits<char,int>::copy(...)            __msvc_string_view.hpp:219
#3 std::basic_string::_Construct<2,char const*>(...) xstring:951
#4 std::basic_string::basic_string(basic_string const&) xstring:768   <- copy ctor
#5 recti::TreeNode::TreeNode(string const&, Point<int,int> const&) dme_algorithm.hpp:84
#6 recti::DMEAlgorithm::build_merging_tree(...)       dme_algorithm.cpp:143  <- tree.add(TreeNode(id, pos))
#7 recti::DMEAlgorithm::build_merging_tree(...)       dme_algorithm.cpp:139
#8 recti::DMEAlgorithm::build_clock_tree(...)         dme_algorithm.cpp:97
```

`memcpy` from address `0x000000000000` (size 10) while copying the `name` member of a `TreeNode` — i.e. the source `std::string` `id` was corrupt.

### 3.4 Instrumentation (the smoking gun)

Temporary `std::ofstream` logging was inserted in `build_merging_tree` to inspect the string immediately after creation:

```
[dbg0] node_id=0 id.size=15 id.data=0000000000000000 &id=000000F9169EC2B0 node_ids.size=2 vertical=1 left=2 right=1
[dbg1] pos=(50,100) tree.size=3
```

`fmt::format("n{}", 0)` — expected a 2-char string `"n0"` — returned an object reading as **`size=15, data=nullptr`**. The `std::string` was never validly constructed from the caller's point of view.

Replacing `fmt::format("n{}", node_id++)` with `"n" + std::to_string(node_id)` **made the DME test pass** → `fmt::format` was confirmed as the corruption source.

### 3.5 Confirming the DLL link

`dumpbin /dependents build\Debug\RectiTests.exe` showed:

```
spdlog.dll
fmt.dll
```

And the CMake cache referenced the conda **Release-only** package config:

```
D:/scoop/apps/miniconda3/current/Library/lib/cmake/fmt/fmt-targets-release.cmake
D:/scoop/apps/miniconda3/current/envs/cpp2026/Library/lib/cmake/spdlog/spdlogConfigTargets-release.cmake
```

---

## 4. Root Cause

### 4.1 The ABI mismatch

MSVC's `std::basic_string` layout depends on `_ITERATOR_DEBUG_LEVEL`:

- **Release / `_ITERATOR_DEBUG_LEVEL=0`**: no debug-iterator proxy pointer.
- **Debug / `_ITERATOR_DEBUG_LEVEL=2`**: `_Container_base12` carries a `_Container_proxy*` inside the string object.

Objects created in code compiled at one level are **misread by code compiled at the other level** — size, capacity, and data-pointer fields land at different offsets.

### 4.2 The chain of events

1. `find_package(fmt CONFIG QUIET)` on this machine resolves to **conda's prebuilt fmt** (shared library, built in Release mode).
2. The project's Debug TU calls `fmt::format(...)`, which executes **inside `fmt.dll`** (Release ABI) and returns a `std::string` by value.
3. The returned object was constructed with the **Release layout**, but the Debug caller interprets it with the **Debug layout** → garbage `size`/`data` (`15` / `nullptr` for `"n0"`).
4. Any subsequent copy (e.g. `TreeNode::TreeNode(const std::string&, ...)` → `name(name)`) dereferences the corrupt pointer → **SIGSEGV** (DME tests) or an unhandled failure that doctest reports as **"Terminate handler called"** (router tests, which go through `log_with_spdlog` / spdlog's own fmt usage).

The same incompatibility applies to `spdlog.dll` (also a conda Release shared library, `SPDLOG_SHARED_LIB` + `SPDLOG_COMPILED_LIB` + `SPDLOG_FMT_EXTERNAL`), which is why the router tests crashed at their logging/formatting boundary.

### 4.3 Why Debug-only and why conda-specific

- **Debug-only**: only Debug sets `_ITERATOR_DEBUG_LEVEL=2`; Release↔Release is consistent.
- **Conda-specific**: conda ships **Release-only** fmt/spdlog (no Debug variant; the `fmt` CMake config is `fmt-targets-release.cmake`). A properly built system fmt with a matching Debug variant would not exhibit this.
- **CI unaffected**: GitHub Actions `windows-latest` has no conda, so CPM builds fmt/spdlog from source with the same `_ITERATOR_DEBUG_LEVEL` as the project.

---

## 5. The Fix

**File**: `CMakeLists.txt` (only change)

```cmake
# fmt is a transitive dependency of spdlog. Prefer a system-installed fmt when available.
# On MSVC, skip prebuilt system packages (e.g. conda): they are Release-only shared
# libraries whose std::string ABI (_ITERATOR_DEBUG_LEVEL=0) mismatches Debug builds
# (_ITERATOR_DEBUG_LEVEL=2), corrupting std::strings returned by fmt::format. Build fmt
# from source on MSVC so the ABI always matches (same as CI's windows-latest runner).
if(NOT MSVC)
  find_package(fmt CONFIG QUIET)
  if(fmt_FOUND)
    message(STATUS "Found system fmt: ${fmt_DIR}")
    set(CPM_fmt_ADDED YES)
  endif()
endif()

if(NOT fmt_FOUND)
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )
endif()
```

The identical pattern was applied to `spdlog` (prefer system only when `NOT MSVC`, otherwise CPM source build with `SPDLOG_INSTALL YES` + `SPDLOG_FMT_EXTERNAL YES`).

**Effect**: on MSVC, fmt/spdlog are compiled from source within the same CMake configuration, so `_ITERATOR_DEBUG_LEVEL` matches the consuming targets in both Debug and Release. Non-MSVC platforms (Ubuntu/macOS) keep preferring system packages — their behavior is unchanged. No application source code was modified (the DME/router code was correct).

A fresh configure was required (`CMakeCache.txt` removal) so the previously cached `fmt_DIR`/`spdlog_DIR` did not short-circuit the new logic.

---

## 6. Verification

### 6.1 Debug build (previously crashing)

```
[doctest] test cases: 154 | 154 passed | 0 failed | 0 skipped
[doctest] assertions: 766 | 766 passed | 0 failed |
[doctest] Status: SUCCESS!
```

- `ctest --test-dir build -C Debug`: **100% tests passed**.
- All 7 previously-crashing tests now pass, including the 5 router suites and both DME tests.
- `dumpbin /dependents` no longer lists `fmt.dll`/`spdlog.dll` (statically linked from source).

### 6.2 Release build

```
[doctest] test cases: 154 | 154 passed | 0 failed | 0 skipped
[doctest] assertions: 766 | 766 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### 6.3 Summary table

| Build | Before fix | After fix |
|---|---|---|
| CMake Debug | 9 passed / 1 failed / 144 skipped (SIGSEGV) | **154/154 passed** (766 assertions) |
| CMake Release | 154/154 passed | **154/154 passed** |
| ctest -C Debug | FAILURE | **100% pass** |
| xmake Release | 154/154 passed | unchanged (separate build path, unaffected) |

---

## 7. Lessons Learned / Recommendations

1. **Do not mix MSVC Debug code with Release-only prebuilt DLLs that return STL objects by value.** Any `std::string`/`std::vector`/`std::function` crossing the boundary will be corrupted due to `_ITERATOR_DEBUG_LEVEL` layout differences.
2. **Prefer source-built dependencies (CPM/FetchContent) for MSVC**, or system packages that ship a matching Debug variant (`fmtd.lib`, `spdlogd.lib`).
3. **Reproduce with AddressSanitizer when cdb is unavailable** — `/fsanitize=address` on MSVC gives exact crash frames; remember the `clang_rt.asan_*` DLL on `PATH`.
4. **Instrument with file logging** (not `std::cout`) when stdout is lost to an ASan abort — `ofstream` + `flush()` survives process termination.
5. **Baseline comparison via `git stash`** cleanly distinguishes pre-existing failures from regressions introduced by an unrelated change (here, the LdsGen→Lds swap).

---

## 8. Appendix: Files Touched

- `CMakeLists.txt` — the fix (fmt/spdlog system-package gating on MSVC).
- No application/test source changes were required for this crash fix.

The working tree also contains the separate, already-verified LdsGen→Lds migration (16 files) that was in flight when this crash was first observed; it is unrelated to this defect.
