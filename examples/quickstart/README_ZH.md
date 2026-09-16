# SpineRuntime 快速开始

[English](README.md) | 简体中文

本示例启动一个 8 tile 的网格，每个 tile 把自己 ID 的平方写入输出缓冲区，宿主端等待任务完成并校验结果。

## 文件

- `demo.cpp`：demo 源码。
- `CMakeLists.txt`：链接 `spine-runtime::spert` 的 CMake 工程。

## 方式一：使用预编译 SDK 交叉编译

该方式适用于已安装 RISC-V 交叉工具链的开发主机。解压从
[Releases](https://github.com/spacemit-com/spine-runtime/releases) 页面获取的
`spine-runtime.xxx.tar.gz`，在仓库根目录将 CMake 同时指向交叉工具链文件和解压后的 SDK，然后编译 demo：

```bash
tar -xf spine-runtime.xxx.tar.gz
export SPINE_RUNTIME_SDK=/absolute/path/to/extracted/spine-runtime

cmake -S examples/quickstart -B build-k3 \
  -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/riscv64-toolchain.cmake \
  -DCMAKE_PREFIX_PATH="$SPINE_RUNTIME_SDK"
cmake --build build-k3 --parallel
```

压缩包名称、解压目录、工具链文件和 sysroot 由具体 SDK 版本及工具链决定，请将占位路径替换为 SDK 交付信息中的实际路径。运行程序前，K3 必须提供 ABI 兼容的 `libspert`。下面的示例安装板端软件包；请根据 SDK 发布说明确认其 `libspert` SONAME 和 API 兼容性。如果不兼容，应部署同一 SDK 压缩包中的 `libspert`，并在板端配置动态库搜索路径。

```bash
export K3_HOST=root@k3-board-address
ssh "$K3_HOST" 'apt install -y spacemit-runtime'
scp build-k3/spine_quickstart "$K3_HOST":/tmp/
ssh "$K3_HOST" /tmp/spine_quickstart
```

上述交叉编译流程用于说明集成方式，未作为本仓库文档验证的一部分实际执行。

## 方式二：在 K3 上原生编译

在运行 Bianbu 的 K3 板子上安装 SpineRuntime 和原生编译工具。请以 `root` 执行安装命令，或在命令前添加 `sudo`；如果 APT 软件包索引已经过期，请先刷新索引。然后在仓库根目录编译 demo：

```bash
apt install -y spacemit-runtime g++ pkg-config
g++ -std=c++17 examples/quickstart/demo.cpp \
  $(pkg-config --cflags --libs spine-runtime) \
  -pthread -o spine_quickstart
./spine_quickstart
```

上述流程已在安装 `spacemit-runtime 0.6.0+1` 的 Bianbu 4.0.2 riscv64 K3 板子上验证，实测输出为：

```text
SpineRuntime quickstart passed on 8 core(s)
```

输出的核数是实际授予给 Stream 的核数；当其他 Stream 或进程正在占用 K3 计算核时，该数值可能不同。
