# Supported Backend Modes

English | [简体中文](backends_ZH.md)

SpineRuntime supports both automatic and explicit selection:

```cpp
// Automatically detect real hardware; use generic/qemu if none is detected.
spert::BackendHandle automatic = spert::default_backend();

// Explicitly obtain a Backend by name.
spert::BackendHandle k1 = spert::backend("spacemit-k1");
spert::BackendHandle k3 = spert::backend("spacemit-k3");
spert::BackendHandle generic = spert::backend("generic/qemu");
```

| Backend Name | Target Platform / Purpose | Default Compute-Core Topology | Per-Core Shared Buffer | Selection Method |
|---|---|---|---:|---|
| `spacemit-k1` | SpacemiT K1 AI compute cores | CC Core `{0, 1, 2, 3}` | 128 KiB | Selected automatically on K1 hardware, or explicitly by name. |
| `spacemit-k3` | SpacemiT K3 A100 AI compute cores | CC Core `{8, 9, 10, 11, 12, 13, 14, 15}` | 384 KiB | Selected automatically on K3 hardware, or explicitly through `spacemit-k3` / `spacemit`. |
| `spacemit-k3-x100` | Compatible execution mode on the K3 X100 core set | Core `{4, 5, 6, 7}` | 384 KiB equivalent fallback | Selected explicitly by name, or used as a generic simulation target. |
| `generic/qemu` | Functional development and validation on standard Linux, qemu-user, or systems without hardware | Logical Core `0..N-1`, determined by the simulation target | Follows the simulation target | Selected automatically when no hardware is detected, or explicitly through `generic` / `generic/qemu`. |

When no physical SpacemiT platform is detected, generic/qemu simulates the core count and hardware information of `spacemit-k3` by default. Before starting the process, set `SPERT_BACKEND` to select `spacemit-k1`, `spacemit-k3`, or `spacemit-k3-x100` as the simulation target. Physical-platform detection takes precedence over this environment variable.

generic/qemu is intended for validating the programming model, scheduling flow, and API integration; it does not represent the performance, core-affinity behavior, or TCM behavior of physical AI compute cores. Applications can query the final visible core count, shared-buffer capacity, vector length, and architecture ID at runtime through `backend_info()` instead of hard-coding platform parameters in application logic.
