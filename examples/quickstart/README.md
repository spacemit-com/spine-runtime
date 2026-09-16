# SpineRuntime Quickstart

English | [简体中文](README_ZH.md)

This example launches an eight-tile grid; each tile writes the square of
its tile ID to an output buffer, and the host waits for completion and
verifies the result.

## Files

- `demo.cpp`: the demo source.
- `CMakeLists.txt`: a CMake project linking `spine-runtime::spert`.

## Option 1: Cross-compile with a prebuilt SDK

Use this path on a development host that has a RISC-V cross toolchain.
Extract the released `spine-runtime.xxx.tar.gz` from the
[Releases](https://github.com/spacemit-com/spine-runtime/releases) page,
then point CMake at both the toolchain file and the extracted SDK and
build the demo from the repository root:

```bash
tar -xf spine-runtime.xxx.tar.gz
export SPINE_RUNTIME_SDK=/absolute/path/to/extracted/spine-runtime

cmake -S examples/quickstart -B build-k3 \
  -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/riscv64-toolchain.cmake \
  -DCMAKE_PREFIX_PATH="$SPINE_RUNTIME_SDK"
cmake --build build-k3 --parallel
```

The archive name, extraction directory, toolchain file, and sysroot are
release- and toolchain-specific; replace the placeholders with paths from
your SDK delivery. Before running the executable, K3 must provide an
ABI-compatible `libspert`. The following example installs the board
package; confirm its `libspert` SONAME and API compatibility against the
SDK release notes. If they do not match, deploy `libspert` from the same
SDK archive and configure the board's dynamic-library search path instead.

```bash
export K3_HOST=root@k3-board-address
ssh "$K3_HOST" 'apt install -y spacemit-runtime'
scp build-k3/spine_quickstart "$K3_HOST":/tmp/
ssh "$K3_HOST" /tmp/spine_quickstart
```

This cross-compilation outline is intended as an integration template and
is not executed as part of this repository's documentation verification.

## Option 2: Build natively on K3

On a K3 board running Bianbu, install SpineRuntime and the native build
tools. Run the package command as `root`, or prefix it with `sudo`;
refresh the APT package index first if it is stale. Then build the demo
from the repository root:

```bash
apt install -y spacemit-runtime g++ pkg-config
g++ -std=c++17 examples/quickstart/demo.cpp \
  $(pkg-config --cflags --libs spine-runtime) \
  -pthread -o spine_quickstart
./spine_quickstart
```

This workflow was verified on a Bianbu 4.0.2 riscv64 K3 board with
`spacemit-runtime 0.6.0+1`. The observed output was:

```text
SpineRuntime quickstart passed on 8 core(s)
```

The reported core count is the number actually granted to the Stream and
can vary when other Streams or processes are using K3 compute cores.
