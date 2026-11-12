# AIDMA Double-Buffered Copy

English | [简体中文](README_ZH.md)

This example copies eight 64 KiB chunks from device memory into a core's shared
buffer and reduces each one, using two buffers so that the copy of the next
chunk runs while the current chunk is being processed. One buffer would force a
wait before every chunk and leave no overlap.

Because the effect comes from the two transfers sharing time with the compute,
this example is worth running only on hardware with a DMA engine (AIDMA): K3 or
K3-X100. On any other backend it prints a `SKIP` line and exits 0, since
`Alloc(size, spert::kMemMMZ)` returns null there.

## Files

- `overlap.cpp`: the example source.
- `CMakeLists.txt`: a CMake project linking `spine-runtime::spert`.

## Cross-compiling against the prebuilt SDK

Extract the `spine-runtime.xxx.tar.gz` from the
[Releases](https://github.com/spacemit-com/spine-runtime/releases) page, then
point CMake at both the toolchain file and the extracted SDK:

```bash
tar -xf spine-runtime.xxx.tar.gz
export SPINE_RUNTIME_SDK=/absolute/path/to/extracted/spine-runtime

cmake -S examples/aidma-overlap -B build-aidma \
  -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/riscv64-toolchain.cmake \
  -Dspine-runtime_DIR="$SPINE_RUNTIME_SDK/lib/cmake/spine-runtime"
cmake --build build-aidma --parallel
```

`spine-runtime_DIR` points straight at the package config directory. That form
is used here rather than `CMAKE_PREFIX_PATH` because a cross toolchain file
usually restricts `CMAKE_FIND_ROOT_PATH`, which filters prefix-path package
lookups; naming the config directory sidesteps it.

Copy the binary and the SDK's `libspert` to the board, then run it with the
library search path pointing at both:

```bash
scp build-aidma/spine_aidma_overlap "$K3_HOST":/tmp/
scp "$SPINE_RUNTIME_SDK"/lib/libspert.so* "$K3_HOST":/tmp/
ssh "$K3_HOST" 'LD_LIBRARY_PATH=/tmp /tmp/spine_aidma_overlap'
```

## Building natively on K3

```bash
apt install -y spacemit-runtime g++ pkg-config
g++ -std=c++17 examples/aidma-overlap/overlap.cpp \
  $(pkg-config --cflags --libs spine-runtime) \
  -pthread -o spine_aidma_overlap
./spine_aidma_overlap
```

Expected output on a machine with AIDMA:

```text
aidma_overlap: 8 chunks x 64 KiB, checksum ok
```

The checksum covers every chunk, so a transfer that is short, shifted, or
skipped makes the program exit non-zero instead of printing this line.

## Notes

A single 2D strided transfer is not supported by the engine. To handle strided
data, either issue one copy per row or repack it into a contiguous block on the
host first. See [API and Feature Overview](../../docs/api-reference.md) for the
full set of device-memory rules.
