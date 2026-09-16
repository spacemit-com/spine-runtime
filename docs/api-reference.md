# API and Feature Overview

English | [简体中文](api-reference_ZH.md)

### Backend and Runtime Queries

| API | Function |
|---|---|
| `backend(name)` | Obtains or initializes a Backend by name; returns an invalid handle on failure. |
| `default_backend()` | Obtains the runtime-selected default Backend. |
| `BackendHandle::valid()` | Reports whether the Backend handle is valid. |
| `backend_info()` | Queries the default Backend's `BackendInfo`. |
| `backend_info(handle)` | Queries the specified Backend's `BackendInfo`. |
| `runtime_stats()` | Obtains a resource snapshot for the default Backend; intended for diagnostics and tests, not recommended for kernel hot paths. |

### Streams and Task Submission

| API | Function |
|---|---|
| `Stream()` / `Stream(n_cores)` | Creates a Stream on the default Backend. |
| `Stream(handle, config)` | Creates a configured Stream on the specified Backend. |
| `Stream::valid()` | Reports whether the Stream successfully acquired runtime resources. |
| `Stream::core_count()` | Returns the number of compute cores actually granted to the Stream. |
| `Stream::launch(grid, fn, args...)` | Launches a Tile-SPMD task and returns a `Future`. |
| `Stream::make_barrier(total)` | Creates a Stream-level Barrier with the specified participant count. |
| `Stream::make_event()` | Creates a manual-reset Stream-level Event. |

### Futures and Synchronization Objects

| API | Function |
|---|---|
| `Future::valid()` | Reports whether the completion handle is valid. |
| `Future::sync()` | Waits for the task to enter a terminal state. |
| `Future::sync(timeout_ms)` | Waits within the specified timeout budget. |
| `Future::sync_all(futures)` | Waits for a group of tasks and returns the first non-`Ok` status. |
| `Future::sync_all(futures, timeout_ms)` | Waits within one timeout budget shared across the group. |
| `Event::signal()` | Sets the Event and wakes waiting tiles. |
| `Event::reset()` | Returns the Event to the unset state. |
| `Barrier::valid()` / `Event::valid()` | Reports whether the synchronization-object handle is valid. |

`Future`, `Barrier`, and `Event` are copyable and movable shared RAII handles. The runtime reclaims the underlying object after the last handle is released and its lifetime conditions are satisfied.

### Context

| API | Function |
|---|---|
| `program_id(dim)` / `grid_dim(dim)` | Queries the tile coordinates and grid dimensions. |
| `yield()` | Cooperatively yields the current compute core. |
| `sync()` | Synchronizes all tiles in the current launch. |
| `barrier(barrier)` | Waits on a Barrier created by the same Stream. |
| `wait(event)` | Waits on an Event created by the same Stream. |
| `spawn(fn, args...)` | Spawns a child task in the current Stream and returns a `Future`. |
| `join(future)` | Cooperatively waits for a Future from the same Stream. |
| `prefetch(addr)` | Issues a best-effort read-prefetch hint. |
| `shared_buffer()` | Obtains a view of the current compute core's complete shared buffer. |
| `alloc_shared(bytes, alignment)` | Cooperatively allocates a shared-buffer slice. |
| `free_shared(view)` | Releases a shared-buffer slice allocated by the current tile. |

### Status Codes

| `Status` | Meaning |
|---|---|
| `Ok` | The operation completed successfully. |
| `InvalidArg` | A handle, pointer, dimension, or argument is invalid. |
| `Deadlock` | The current Stream has entered a confirmed cooperative deadlock state. |
| `Timeout` | The caller-provided wait deadline has expired. |
| `BackendUnavailable` | The Backend is unavailable, failed to initialize, or is shutting down. |
| `Unsupported` | The current context or platform does not support the operation. |
| `NoMem` | Runtime resource allocation failed. |
| `Failed` | A general error occurred that is not classified under another status. |
| `Cancelled` | A task was cancelled while its Stream was shutting down, or an operation used a synchronization object belonging to a closed Stream. |
| `CrossStream` | A Stream-level object was used from another Stream. |

Use `spert::to_string(status)` to obtain the stable English name of a status.

### Compiler Integration ABI

`spert_abi.h` provides an `extern "C"` ABI for compiler-generated code, MLIR Runner, and similar integration scenarios. Its primary symbols include:

| Interface Group | Function |
|---|---|
| `spine_get_*` | Queries the default Backend's architecture ID, shared-buffer capacity, vector length, and compute-core count. |
| `spine_require_stream*` / `spine_release_stream` | Obtains and releases a Stream handle. |
| `spine_parallel_dispatch_*` | Synchronously launches a 1D, 2D, or 3D grid. |
| `spine_parallel_dispatch_*_async` / `spine_parallel_sync` | Asynchronously launches a grid and consumes its completion token. |
| `spine_grid` | Queries the current tile's coordinate in the specified dimension. |
| `spine_thread_tcm_malloc` / `spine_thread_tcm_free` | Allocates and releases a tile's per-core shared-memory slice. |
| `spine_alloc_with_id` / `spine_free_with_id` | Overridable ID-based memory-allocation hooks. |

In addition, `spert_abi.h` provides the header-inline helper function `spine_thread_cpu_relax()`, which issues an architecture-specific CPU-relax hint. This function is not a linkable ABI symbol exported by the shared library.

Application development should prefer `spert.hpp`. Only compilers, code generators, or existing ABI adaptation layers need to use `spert_abi.h` directly.

### Closed-Source Package Integration

The SDK runtime requires C++17. When using its CMake package configuration, add the SDK root directory to `CMAKE_PREFIX_PATH`:

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

Compiler and linker flags are also available through `pkg-config`:

```bash
export PKG_CONFIG_PATH=/path/to/spine-runtime/lib/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
pkg-config --cflags --libs spine-runtime
```

