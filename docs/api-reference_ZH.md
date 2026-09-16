# API 与功能介绍

[English](api-reference.md) | 简体中文

### Backend 与运行时查询

| API | 功能 |
|---|---|
| `backend(name)` | 按名称获取或初始化 Backend，失败时返回无效句柄。 |
| `default_backend()` | 获取运行时自动选择的默认 Backend。 |
| `BackendHandle::valid()` | 判断 Backend 句柄是否有效。 |
| `backend_info()` | 查询默认 Backend 的 `BackendInfo`。 |
| `backend_info(handle)` | 查询指定 Backend 的 `BackendInfo`。 |
| `runtime_stats()` | 获取默认 Backend 的资源快照，适用于诊断和测试，不建议用于 kernel 热路径。 |

### Stream 与任务提交

| API | 功能 |
|---|---|
| `Stream()` / `Stream(n_cores)` | 在默认 Backend 上创建 Stream。 |
| `Stream(handle, config)` | 在指定 Backend 上按配置创建 Stream。 |
| `Stream::valid()` | 判断 Stream 是否成功取得运行时资源。 |
| `Stream::core_count()` | 返回实际授予给 Stream 的计算核数量。 |
| `Stream::launch(grid, fn, args...)` | 启动 Tile-SPMD 任务并返回 `Future`。 |
| `Stream::make_barrier(total)` | 创建指定参与者数量的 Stream 级 Barrier。 |
| `Stream::make_event()` | 创建手动复位的 Stream 级 Event。 |

### Future 与同步对象

| API | 功能 |
|---|---|
| `Future::valid()` | 判断完成句柄是否有效。 |
| `Future::sync()` | 等待任务进入最终状态。 |
| `Future::sync(timeout_ms)` | 在指定超时预算内等待。 |
| `Future::sync_all(futures)` | 等待一组任务并返回首个非 `Ok` 状态。 |
| `Future::sync_all(futures, timeout_ms)` | 在全组共享的超时预算内等待。 |
| `Event::signal()` | 置位 Event 并唤醒等待 tile。 |
| `Event::reset()` | 将 Event 恢复为未置位状态。 |
| `Barrier::valid()` / `Event::valid()` | 判断同步对象句柄是否有效。 |

`Future`、`Barrier` 和 `Event` 都是可复制、可移动的共享 RAII 句柄；底层对象在最后一个句柄释放且生命周期条件满足后由运行时回收。

### Context

| API | 功能 |
|---|---|
| `program_id(dim)` / `grid_dim(dim)` | 查询 tile 坐标和网格维度。 |
| `yield()` | 协作式让出当前计算核。 |
| `sync()` | 同步当前 launch 的全部 tile。 |
| `barrier(barrier)` | 等待同一 Stream 创建的 Barrier。 |
| `wait(event)` | 等待同一 Stream 创建的 Event。 |
| `spawn(fn, args...)` | 在当前 Stream 派生一个子任务并返回 `Future`。 |
| `join(future)` | 协作式等待同一 Stream 的 Future。 |
| `prefetch(addr)` | 发出尽力而为的读预取提示。 |
| `shared_buffer()` | 获取当前计算核的完整共享缓冲区视图。 |
| `alloc_shared(bytes, alignment)` | 协作式申请共享缓冲区片段。 |
| `free_shared(view)` | 释放当前 tile 申请的共享缓冲区片段。 |

### 状态码

| `Status` | 含义 |
|---|---|
| `Ok` | 操作成功完成。 |
| `InvalidArg` | 句柄、指针、维度或参数无效。 |
| `Deadlock` | 当前 Stream 进入已确认的协作式死锁状态。 |
| `Timeout` | 调用方给定的等待期限已到。 |
| `BackendUnavailable` | Backend 不可用、初始化失败或正在关闭。 |
| `Unsupported` | 当前上下文或平台不支持该操作。 |
| `NoMem` | 运行时资源分配失败。 |
| `Failed` | 发生未归类为其他状态的一般错误。 |
| `Cancelled` | Stream 关闭过程中任务被取消，或操作了已关闭 Stream 所属的同步对象。 |
| `CrossStream` | 从其他 Stream 使用了 Stream 级对象。 |

可使用 `spert::to_string(status)` 获取状态的稳定英文名称。

### 编译器集成 ABI

`spert_abi.h` 为编译器生成代码和 MLIR Runner 等集成场景提供 `extern "C"` ABI，主要符号包括：

| 接口组 | 功能 |
|---|---|
| `spine_get_*` | 查询默认 Backend 的架构 ID、共享缓冲区容量、向量长度和计算核数量。 |
| `spine_require_stream*` / `spine_release_stream` | 获取和释放 Stream 句柄。 |
| `spine_parallel_dispatch_*` | 同步启动 1D、2D 或 3D 网格。 |
| `spine_parallel_dispatch_*_async` / `spine_parallel_sync` | 异步启动网格并消费完成 token。 |
| `spine_grid` | 查询当前 tile 在指定维度上的坐标。 |
| `spine_thread_tcm_malloc` / `spine_thread_tcm_free` | 申请和释放 tile 的每核共享内存片段。 |
| `spine_alloc_with_id` / `spine_free_with_id` | 可覆盖的带 ID 内存分配钩子。 |

此外，`spert_abi.h` 提供头文件内联辅助函数 `spine_thread_cpu_relax()`，用于发出架构相关的 CPU relax 提示；该函数不是动态库导出的可链接 ABI 符号。

应用开发优先使用 `spert.hpp`；只有编译器、代码生成器或既有 ABI 适配层需要直接使用 `spert_abi.h`。

### 闭源包集成

SDK 运行时要求 C++17。使用 CMake 包配置时，将 SDK 根目录加入 `CMAKE_PREFIX_PATH`：

```cmake
cmake_minimum_required(VERSION 3.30)
project(spine_runtime_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spine-runtime CONFIG REQUIRED)

add_executable(spine_runtime_app main.cpp)
target_link_libraries(spine_runtime_app PRIVATE spine-runtime::spert)
```

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/spine-runtime
cmake --build build
```

也可以通过 `pkg-config` 获取头文件和链接参数：

```bash
export PKG_CONFIG_PATH=/path/to/spine-runtime/lib/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
pkg-config --cflags --libs spine-runtime
```

