<div align="center">
    <h1>SpineRuntime</h1>
    <p><strong>
        SpacemiT RISC-V AI 众核执行运行时
    </strong></p>
</div>

[English](README.md) | 简体中文

SpineRuntime（库名 `spert`）面向 SpacemiT RISC-V SoC，负责在宿主通用核与 AI 计算核之间组织并行任务。SDK 以预编译动态库 `libspert`、C++ 公共头文件和编译器集成 ABI 的形式交付；应用只需描述计算网格与 tile kernel，运行时负责 Backend 选择、计算核资源申请、任务调度、同步和资源回收。

## 简介

SpineRuntime 提供 Tile-SPMD（Single Program, Multiple Data）编程模型：一次 `launch` 将同一个 kernel 实例化为一组 tile，每个 tile 处理网格中的一个坐标。应用无需直接管理 AI 计算核线程，可通过统一接口在不同 SpacemiT 平台和 generic/qemu 环境中运行相同的调度代码。

核心特性包括：

- **现代 C++17 API**：使用强类型参数、RAII 句柄和 `enum class Status`，无需手工封装 `void*` 参数。
- **1D/2D/3D Tile-SPMD**：通过 `Grid` 描述并行空间，通过 `Context` 查询 tile 坐标和网格维度。
- **Stream 级多核执行**：每个 `Stream` 持有一组 Backend 授予的计算核资源，提交到该 Stream 的 tile 只在对应资源范围内执行。
- **异步任务与组合等待**：`launch` 返回 `Future`，支持单任务等待、超时等待和多个任务的合并等待。
- **tile 内协作**：提供全网格同步、Barrier、Event、子任务 `spawn/join` 和协作式 `yield`。
- **每核共享缓冲区**：kernel 可访问当前计算核的共享暂存区，也可按需申请和释放 tile 私有片段。
- **多 Backend 共存**：支持自动选择默认 Backend，也支持通过名称获取独立的 `BackendHandle`。
- **稳定集成边界**：模板参数在应用侧完成类型封装，预编译运行时通过非模板接口接收任务；二进制兼容范围以匹配的 SDK 头文件和 `libspert` SONAME 主版本为边界。

整体使用关系如下：

```text
宿主应用 / 编译器生成代码
           │
           ├── C++ API：spert.hpp
           └── 集成 ABI：spert_abi.h
                       │
                 BackendHandle
                       │
          ┌────────────┴────────────┐
        Stream A                  Stream B
          │                         │
     Grid → Tiles              Grid → Tiles
          │                         │
     获授予的 CC Core          获授予的 CC Core
```

## 快速开始

### 前置条件

- 一块运行 Bianbu 的 K3 板子用于原生编译，或一台装有 RISC-V 64 位交叉工具链的开发主机用于交叉编译。
- SpineRuntime SDK：板端的 `spacemit-runtime` 软件包，或 [Releases](https://github.com/spacemit-com/spine-runtime/releases) 页面提供的预编译 `spine-runtime.xxx.tar.gz` 压缩包。
- C++17 编译器，以及用于集成的 CMake 或 `pkg-config`。

### 构建并运行 demo

在运行 Bianbu 的 K3 板子上安装 SpineRuntime 和编译工具，然后在仓库根目录编译 [`examples/quickstart`](examples/quickstart) 中的独立 demo：

```console
sudo apt install -y spacemit-runtime g++ pkg-config
git clone https://github.com/spacemit-com/spine-runtime.git
cd spine-runtime
g++ -std=c++17 examples/quickstart/demo.cpp \
  $(pkg-config --cflags --libs spine-runtime) \
  -pthread -o spine_quickstart
./spine_quickstart
```

预期输出：

```text
SpineRuntime quickstart passed on 8 core(s)
```

输出的核数是实际授予给 Stream 的核数；当其他 Stream 或进程正在占用 K3 计算核时，该数值可能不同。使用预编译 SDK 交叉编译的流程见 [`examples/quickstart/README_ZH.md`](examples/quickstart/README_ZH.md)。

## 文档

| 文档 | 内容 |
|---|---|
| [编程模型](docs/programming-model_ZH.md) | 核心对象、tile kernel 与任务同步 |
| [多核调度模型](docs/scheduling_ZH.md) | Stream、计算核授权、tile 调度与共享缓冲区 |
| [API 与功能介绍](docs/api-reference_ZH.md) | 公共 C++ API、状态码、编译器集成 ABI 与包集成 |
| [支持的 Backend 模式](docs/backends_ZH.md) | Backend 选择、拓扑与 `generic/qemu` 模拟目标 |

## Backend 选择

SpineRuntime 自动选择可用的 Backend。未探测到真实 SpacemiT 平台时使用 `generic/qemu`；可在进程启动前通过 `SPERT_BACKEND` 选择 `spacemit-k1`、`spacemit-k3` 或 `spacemit-k3-x100` 作为模拟目标。真实平台探测结果优先于该环境变量。按名称显式选择及完整 Backend 表见 [支持的 Backend 模式](docs/backends_ZH.md)。

## 许可证

见 [LICENSE](LICENSE)。
