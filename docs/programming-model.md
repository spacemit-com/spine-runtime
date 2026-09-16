# Programming Model

English | [简体中文](programming-model_ZH.md)

### Core Objects

| Object | Role |
|---|---|
| `BackendHandle` | Identifies a Backend instance that remains valid for the lifetime of the process; copyable and non-owning. |
| `BackendInfo` | Describes a Backend's compute-core count, per-core shared-buffer capacity, vector length, and architecture ID. |
| `Grid` | Describes a 1D, 2D, or 3D tile grid; the product of all valid dimensions is the total tile count. |
| `StreamConfig` | Configures the desired compute-core count or preferred physical core IDs. |
| `Stream` | A move-only RAII execution domain that manages Backend-granted compute-core resources and task lifetimes. |
| `Context` | The execution context passed by the runtime to each tile; valid only during that kernel invocation. |
| `Future` | A shared completion handle for a `launch` or `spawn`. |
| `Barrier` / `Event` | Bound to the Stream that created them and used for cooperative synchronization between tiles. |
| `SharedBufferView` | A non-owning view of the current compute core's shared scratchpad. |

### Tile Kernel

A kernel is a user-defined callable whose first parameter must be `spert::Context*`. Remaining arguments are stored by value by `launch`. Pointers can provide access to shared inputs and outputs, but the caller must ensure that the pointed-to data remains valid until the `Future` completes.

Every tile executes the same kernel and determines its assigned data range through these interfaces:

- `program_id(dim)`: Returns the current tile's coordinate in the specified dimension.
- `grid_dim(dim)`: Returns the grid size in the specified dimension.

### Minimal Example

The following example uses `Grid(2, 4)` to divide vector addition across eight tiles and flattens the two-dimensional tile coordinates into a linear index:

```cpp
#include "spert.hpp"

#include <cstdint>
#include <vector>

void vector_add(spert::Context* ctx,
                const float* a,
                const float* b,
                float* output,
                uint32_t size) {
    const uint32_t grid_x = ctx->grid_dim(0);
    const uint32_t grid_y = ctx->grid_dim(1);
    const uint32_t tile_id = ctx->program_id(1) * grid_x + ctx->program_id(0);
    const uint32_t tiles = grid_x * grid_y;

    for (uint32_t i = tile_id; i < size; i += tiles) {
        output[i] = a[i] + b[i];
    }
}

int main() {
    constexpr uint32_t size = 4096;
    std::vector<float> a(size, 1.0f);
    std::vector<float> b(size, 2.0f);
    std::vector<float> output(size, 0.0f);

    spert::Stream stream;
    if (!stream.valid()) {
        return 1;
    }

    spert::Future future = stream.launch(
        spert::Grid(2, 4), vector_add, a.data(), b.data(), output.data(), size);
    if (!future.valid()) {
        return 2;
    }

    const spert::Status status = future.sync();
    return status == spert::Status::Ok ? 0 : 3;
}
```

### Task Dependencies and Synchronization

`launch` is an asynchronous submission interface. Kernels with read/write dependencies should establish an explicit execution order through `Future::sync()`, `Future::sync_all()`, or in-tile cooperation primitives. Independent tasks can be submitted consecutively and then waited on together.

```cpp
auto f0 = stream.launch(spert::Grid(n), kernel0, args0);
auto f1 = stream.launch(spert::Grid(n), kernel1, args1);

spert::Status status = spert::Future::sync_all({f0, f1});
spert::Status timed = spert::Future::sync_all({f0, f1}, 1000);
```

