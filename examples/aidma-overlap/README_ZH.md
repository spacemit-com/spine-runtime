# AIDMA 双缓冲异步拷贝

[English](README.md) | 简体中文

本示例从设备内存分八次拷贝 64 KiB 数据块到核内共享缓冲区，并对每块做归约；使用两块缓冲区，使"下一块的拷贝"与"当前块的计算"同时进行。只用一块缓冲区的话，每块之前都必须等待，重叠就消失了。

效果来自传输与计算共享时间，因此只有在带 DMA 引擎（AIDMA）的硬件上运行才有意义：K3 或 K3-X100。在其他 Backend 上，`Alloc(size, spert::kMemMMZ)` 返回空指针，程序打印一行 `SKIP` 并以 0 退出。

## 文件

- `overlap.cpp`：示例源码。
- `CMakeLists.txt`：链接 `spine-runtime::spert` 的 CMake 工程。

## 使用预编译 SDK 交叉编译

解压从 [Releases](https://github.com/spacemit-com/spine-runtime/releases) 页面获取的
`spine-runtime.xxx.tar.gz`，将 CMake 同时指向交叉工具链文件和解压后的 SDK：

```bash
tar -xf spine-runtime.xxx.tar.gz
export SPINE_RUNTIME_SDK=/absolute/path/to/extracted/spine-runtime

cmake -S examples/aidma-overlap -B build-aidma \
  -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/riscv64-toolchain.cmake \
  -Dspine-runtime_DIR="$SPINE_RUNTIME_SDK/lib/cmake/spine-runtime"
cmake --build build-aidma --parallel
```

这里用 `spine-runtime_DIR` 直接指向包配置目录，而没有用 `CMAKE_PREFIX_PATH`：交叉工具链文件通常会把 `CMAKE_FIND_ROOT_PATH` 限制住，导致按前缀路径查找包被过滤掉；直接指定配置目录可以绕开这个问题。

将可执行文件和 SDK 中的 `libspert` 一起拷到板端，运行时让动态库搜索路径同时包含两者：

```bash
scp build-aidma/spine_aidma_overlap "$K3_HOST":/tmp/
scp "$SPINE_RUNTIME_SDK"/lib/libspert.so* "$K3_HOST":/tmp/
ssh "$K3_HOST" 'LD_LIBRARY_PATH=/tmp /tmp/spine_aidma_overlap'
```

## 在 K3 上原生编译

```bash
apt install -y spacemit-runtime g++ pkg-config
g++ -std=c++17 examples/aidma-overlap/overlap.cpp \
  $(pkg-config --cflags --libs spine-runtime) \
  -pthread -o spine_aidma_overlap
./spine_aidma_overlap
```

在带 AIDMA 的机器上，预期输出为：

```text
aidma_overlap: 8 chunks x 64 KiB, checksum ok
```

校验和覆盖每一个数据块，因此拷贝长度不足、位置错位或被跳过时，程序会以非零码退出，而不会打印这一行。

## 说明

引擎不支持单次 2D 跨行传输。处理带跨行的数据时，可以按行逐次发起拷贝，或先在宿主侧整理成连续块。完整的设备内存规则见 [API 与功能总览](../../docs/api-reference_ZH.md)。
