# IFLOW.md - 物理设计 C++ 代码库

## 项目概述

这是一个现代 C++ 物理设计代码库，专注于 VLSI（超大规模集成电路）物理设计中的几何算法。项目名为 "Recti"，提供了处理矩形、多边形、线段、路由树等几何对象的高效数据结构和算法。该库采用现代 CMake 实践，支持模块化构建，并包含测试套件、文档生成和持续集成。

主要特性包括：
- 现代 CMake 实践，支持单头文件库和任意规模的项目
- 库代码和可执行文件代码的清晰分离
- 集成测试套件
- 通过 GitHub Actions 进行持续集成
- 通过 codecov 进行代码覆盖率分析
- 通过 clang-format 和 cmake-format 强制代码格式化
- 通过 CPM.cmake 进行可重现的依赖管理
- 可安装的目标，具有自动版本信息和头文件生成
- 通过 Doxygen 和 GitHub Pages 自动生成文档和部署
- 支持 sanitizer 工具等

## 核心组件

### 几何数据结构
- **Interval<T>**: 表示一维区间
- **Point<T, U>**: 表示二维点，可以是普通点或区间点
- **Rectangle<T>**: 表示矩形（矩形多边形）
- **HSgment<T>/VSegment<T>**: 表示水平/垂直线段
- **RPolygon<T>**: 表示矩形多边形
- **Polygon<T>**: 表示一般多边形

### 路由算法
- **GlobalRouter**: 全局路由器，能够构建路由树
- **GlobalRoutingTree**: 全局路由树数据结构
- **RoutingNode**: 路由树中的节点（源、终端、Steiner 点）
- 支持简单路由、Steiner 点路由和带约束的路由
- 支持 2D 和 3D 路由可视化

### 其他算法
- **DME (Distributed Multi-value Equilibrium) 算法**: 用于优化物理设计参数
- **Halton 序列生成器**: 用于生成准随机数序列
- **SVG 可视化工具**: 用于生成几何对象和路由树的 SVG 图像

## 构建和运行

### 构建独立目标
```bash
cmake -S standalone -B build/standalone
cmake --build build/standalone
./build/standalone/Recti --help
```

### 构建和运行测试套件
```bash
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# 或直接调用可执行文件：
./build/test/RectiTests
```

### 运行 clang-format
```bash
cmake -S test -B build/test

# 查看更改
cmake --build build/test --target format

# 应用更改
cmake --build build/test --target fix-format
```

### 构建文档
```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# 查看文档
open build/doc/doxygen/html/index.html
```

### 一次性构建所有内容
```bash
cmake -S all -B build
cmake --build build

# 运行测试
./build/test/RectiTests
# 格式化代码
cmake --build build --target fix-format
# 运行独立程序
./build/standalone/Recti --help
# 构建文档
cmake --build build --target GenerateDocs
```

## 开发约定

- **C++ 标准**: C++20
- **代码格式化**: 使用 clang-format 和 cmake-format
- **依赖管理**: 使用 CPM.cmake
- **测试**: 项目包含全面的测试套件
- **头文件包含**: 使用 `#include "filename"` 格式
- **命名空间**: 所有核心功能都在 `recti` 命名空间中

## 项目结构

- `include/recti/`: 头文件目录，包含所有公共接口
- `source/`: 源文件目录
- `test/`: 测试代码
- `standalone/`: 独立可执行文件示例
- `documentation/`: 文档配置
- `cmake/`: CMake 模块和配置文件
- `experiments/`: 实验性代码

## 扩展功能

该项目支持多种扩展功能：
- **Sanitizers**: 地址、内存、未定义行为等检查器
- **静态分析器**: clang-tidy, iwyu, cppcheck
- **Ccache**: 编译缓存以加快构建速度

可以通过 CMake 配置参数启用这些功能，如 `-DUSE_SANITIZER=Address` 或 `-DUSE_STATIC_ANALYZER=clang-tidy`。

## 使用场景

这个库主要用于 VLSI 物理设计中的几何计算，包括：
- 矩形和多边形操作
- 全局路由算法
- 时钟树综合
- 几何约束处理
- 物理设计优化算法