# 支持的 Backend 模式

[English](backends.md) | 简体中文

SpineRuntime 支持自动选择和显式选择两种方式：

```cpp
// 自动探测真实平台；未探测到实板时使用 generic/qemu。
spert::BackendHandle automatic = spert::default_backend();

// 按名称显式获取 Backend。
spert::BackendHandle k1 = spert::backend("spacemit-k1");
spert::BackendHandle k3 = spert::backend("spacemit-k3");
spert::BackendHandle generic = spert::backend("generic/qemu");
```

| Backend 名称 | 目标平台 / 用途 | 默认计算核拓扑 | 每核共享缓冲区 | 选择方式 |
|---|---|---|---:|---|
| `spacemit-k1` | SpacemiT K1 AI 计算核 | CC Core `{0, 1, 2, 3}` | 128 KiB | K1 实板自动选择，或按名称显式选择。 |
| `spacemit-k3` | SpacemiT K3 A100 AI 计算核 | CC Core `{8, 9, 10, 11, 12, 13, 14, 15}` | 384 KiB | K3 实板自动选择，或使用 `spacemit-k3` / `spacemit` 显式选择。 |
| `spacemit-k3-x100` | K3 X100 核集的兼容执行模式 | Core `{4, 5, 6, 7}` | 384 KiB 等价回退区 | 按名称显式选择，或作为 generic 模拟目标。 |
| `generic/qemu` | 普通 Linux、qemu-user 和无实板环境下的功能开发与验证 | 逻辑 Core `0..N-1`，由模拟目标决定 | 跟随模拟目标 | 未探测到实板时自动选择，或使用 `generic` / `generic/qemu` 显式选择。 |

在未探测到真实 SpacemiT 平台时，generic/qemu 默认模拟 `spacemit-k3` 的核数量与硬件信息。可在进程启动前通过 `SPERT_BACKEND` 选择 `spacemit-k1`、`spacemit-k3` 或 `spacemit-k3-x100` 作为模拟目标。真实平台探测结果优先于该环境变量。

generic/qemu 用于验证编程模型、调度流程和 API 集成，不代表真实 AI 计算核的性能、绑核效果或 TCM 行为。应用可通过 `backend_info()` 在运行时查询最终可见的核数量、共享缓冲区容量、向量长度和架构 ID，避免把平台参数写死在业务代码中。
