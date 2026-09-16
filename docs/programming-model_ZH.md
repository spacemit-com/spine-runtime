# 编程模型

[English](programming-model.md) | 简体中文

### 核心对象

| 对象 | 作用 |
|---|---|
| `BackendHandle` | 标识一个进程生命周期内有效的 Backend 实例，可复制、非拥有。 |
| `BackendInfo` | 描述 Backend 的计算核数量、每核共享缓冲区容量、向量长度和架构 ID。 |
| `Grid` | 描述 1–3 维 tile 网格，所有有效维度的乘积为 tile 总数。 |
| `StreamConfig` | 配置期望的计算核数量或首选物理核 ID。 |
| `Stream` | move-only RAII 执行域，管理 Backend 授予的计算核资源和任务生命周期。 |
| `Context` | 运行时传给每个 tile 的执行上下文，仅在本次 kernel 调用期间有效。 |
| `Future` | 一次 `launch` 或 `spawn` 的共享完成句柄。 |
| `Barrier` / `Event` | 绑定到创建它们的 Stream，用于 tile 间协作同步。 |
| `SharedBufferView` | 指向当前计算核共享暂存区的非拥有视图。 |

### Tile kernel

kernel 是用户定义的可调用对象，第一个参数必须是 `spert::Context*`。其余参数由 `launch` 按值保存；指针可用于访问共享输入和输出，但调用方必须保证指针指向的数据在 `Future` 完成前保持有效。

每个 tile 执行相同的 kernel，通过以下接口确定自己负责的数据范围：

- `program_id(dim)`：返回当前 tile 在指定维度上的坐标；
- `grid_dim(dim)`：返回指定维度的网格大小。

### 最小示例

下面的示例用 `Grid(2, 4)` 把向量加法划分为 8 个 tile，并将二维 tile 坐标展平为线性索引：

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

### 任务依赖与同步

`launch` 是异步提交接口。存在读写依赖的 kernel 应通过 `Future::sync()`、`Future::sync_all()` 或 tile 内协作原语显式建立执行顺序；没有依赖的任务可以先连续提交，再统一等待。

```cpp
auto f0 = stream.launch(spert::Grid(n), kernel0, args0);
auto f1 = stream.launch(spert::Grid(n), kernel1, args1);

spert::Status status = spert::Future::sync_all({f0, f1});
spert::Status timed = spert::Future::sync_all({f0, f1}, 1000);
```

