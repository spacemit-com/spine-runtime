<div align="center">
    <h1>Spine-Runtime</h1>
    <p><strong>
        SpacemiT RISC-V AI Many-Core Execution Runtime
    </strong></p>
</div>

English | [简体中文](README_ZH.md)

Spine-Runtime (library name: `spert`) targets SpacemiT RISC-V SoCs and
coordinates parallel tasks between general-purpose host cores and AI
compute cores. The SDK is distributed as the prebuilt `libspert` shared
library, public C++ headers, and a compiler-integration ABI. Applications
describe only the compute grid and tile kernel; the runtime handles
Backend selection, compute-core resource allocation, task scheduling,
synchronization, and resource reclamation.

## Overview

Spine-Runtime provides a Tile-SPMD (Single Program, Multiple Data)
programming model: each `launch` instantiates the same kernel as a set of
tiles, with every tile processing one coordinate in the grid. Applications
do not need to manage AI compute-core threads directly. The same
scheduling code runs across different SpacemiT platforms and generic/qemu
environments through a unified interface.

Key features include:

- **Modern C++17 API**: Uses strongly typed parameters, RAII handles, and `enum class Status`, eliminating the need to manually wrap `void*` arguments.
- **1D/2D/3D Tile-SPMD**: Describes the parallel space with `Grid` and queries tile coordinates and grid dimensions through `Context`.
- **Stream-level multicore execution**: Each `Stream` owns a set of compute-core resources granted by the Backend. Tiles submitted to that Stream run only within the corresponding resource set.
- **Asynchronous tasks and combined waits**: `launch` returns a `Future`, supporting waits for a single task, timed waits, and combined waits for multiple tasks.
- **In-tile cooperation**: Provides whole-grid synchronization, Barriers, Events, child-task `spawn/join`, and cooperative `yield`.
- **Per-core shared buffers**: A kernel can access the current compute core's shared scratchpad or allocate and release tile-private slices on demand.
- **Multiple coexisting Backends**: Supports automatic selection of the default Backend and access to independent `BackendHandle` instances by name.
- **Stable integration boundary**: Template arguments are wrapped into types on the application side, while the prebuilt runtime receives tasks through non-template interfaces. Binary compatibility is bounded by matching SDK headers and the `libspert` SONAME major version.

The overall usage relationship is as follows:

```text
Host application / compiler-generated code
           │
           ├── C++ API: spert.hpp
           └── Integration ABI: spert_abi.h
                       │
                 BackendHandle
                       │
          ┌────────────┴────────────┐
        Stream A                  Stream B
          │                         │
     Grid → Tiles              Grid → Tiles
          │                         │
     Granted CC Cores          Granted CC Cores
```

## Getting started

### Prerequisites

- A SpacemiT K3 board running Bianbu, or a RISC-V 64-bit cross-compilation
  environment.
- A C++17 compiler with CMake or `pkg-config`.
- SpineRuntime from the `spacemit-runtime` platform package or a prebuilt SDK
  from [Releases](https://github.com/spacemit-com/spine-runtime/releases).

`spacemit-runtime` package availability depends on the Bianbu source: K3 has it
in daily and versioned sources, while K1 may only have it in the daily source
until the next Bianbu release. If APT cannot locate it, use the prebuilt SDK.

GitHub `Source code` archives are documentation snapshots, not SDK packages.

### Quickstart

On a K3 board running Bianbu, install SpineRuntime and the build tools,
then build the standalone demo in
[`examples/quickstart`](examples/quickstart) from the repository root:

```console
sudo apt install -y spacemit-runtime g++ pkg-config
git clone https://github.com/spacemit-com/spine-runtime.git
cd spine-runtime
g++ -std=c++17 examples/quickstart/demo.cpp \
  $(pkg-config --cflags --libs spine-runtime) \
  -pthread -o spine_quickstart
./spine_quickstart
```

Expected output:

```text
SpineRuntime quickstart passed on 8 core(s)
```

The reported core count is the number actually granted to the Stream and
can vary when other Streams or processes are using K3 compute cores. For
cross-compilation with a prebuilt SDK, see
[`examples/quickstart/README.md`](examples/quickstart/README.md).

## Documentation

| Document | Description |
|---|---|
| [Programming Model](docs/programming-model.md) | Core objects, tile kernels, and task synchronization |
| [Multicore Scheduling Model](docs/scheduling.md) | Streams, compute-core grants, tile scheduling, and shared buffers |
| [API and Feature Overview](docs/api-reference.md) | Public C++ API, status codes, compiler-integration ABI, and package integration |
| [Supported Backend Modes](docs/backends.md) | Backend selection, topology, and the `generic/qemu` simulation target |

## Backend selection

SpineRuntime selects an available Backend automatically. When no physical
SpacemiT platform is detected, `generic/qemu` is used; set `SPERT_BACKEND`
before starting the process to select `spacemit-k1`, `spacemit-k3`, or
`spacemit-k3-x100` as the simulation target. Physical-platform detection
takes precedence over this environment variable. See
[Supported Backend Modes](docs/backends.md) for explicit selection by name
and the full Backend table.

## License

See [LICENSE](LICENSE).
