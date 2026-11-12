#include "spert.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>

// Double-buffered copy and compute.
//
// A tile copies chunk N+1 from device memory while it works on chunk N, so the
// transfer and the arithmetic share the elapsed time instead of running back to
// back. One buffer would force a wait before every chunk and remove the overlap
// entirely.
//
// The kernel skips itself on backends without device memory, which is most
// boards; build and run it on K3 or K3-X100 to see the effect.

namespace {

constexpr size_t kChunkBytes = 64 * 1024;
constexpr size_t kChunks     = 8;

// Stand-in for real arithmetic. Sized so the work per chunk is in the same
// range as the transfer; a much cheaper kernel would finish before the copy and
// hide the overlap it is supposed to demonstrate.
uint64_t reduce_chunk(const void * data, size_t bytes) {
    const uint64_t * words = static_cast<const uint64_t *>(data);
    const size_t     count = bytes / sizeof(uint64_t);
    uint64_t         sum   = 0;
    for (size_t i = 0; i < count; ++i) {
        sum += words[i];
    }
    return sum;
}

// The value reduce_chunk() must produce for chunk `index`, computed the same way
// but without involving the device, so a wrong or short transfer shows up.
uint64_t expected_chunk(size_t index) {
    uint64_t sum = 0;
    for (size_t i = 0; i < kChunkBytes / sizeof(uint64_t); ++i) {
        const uint64_t word = (i * 31u) + (index * 97u) + 7u;
        sum += word;
    }
    return sum;
}

void fill_chunk(uint8_t * dst, size_t index) {
    for (size_t i = 0; i < kChunkBytes / sizeof(uint64_t); ++i) {
        const uint64_t word = (i * 31u) + (index * 97u) + 7u;
        std::memcpy(dst + i * sizeof(uint64_t), &word, sizeof(word));
    }
}

void overlap_kernel(spert::Context * ctx, uint8_t * ddr, uint64_t * result) {
    // Both buffers come from this tile's shared memory. Allocating inside the
    // tile is required: the slice belongs to the core the tile runs on, and the
    // copy target must be reachable from that core.
    spert::SharedBufferView buf0 = ctx->alloc_shared(kChunkBytes);
    spert::SharedBufferView buf1 = ctx->alloc_shared(kChunkBytes);
    if (buf0.data == nullptr || buf1.data == nullptr) {
        ctx->free_shared(buf0);
        ctx->free_shared(buf1);
        return;
    }
    spert::SharedBufferView buffers[2] = { buf0, buf1 };

    // Queue the first chunk and wait, because there is nothing to compute yet.
    spert::CopyFuture inflight = ctx->copy_async(buf0.data, ddr, kChunkBytes);
    if (!inflight.valid()) {
        ctx->free_shared(buf0);
        ctx->free_shared(buf1);
        return;
    }
    inflight.sync();

    uint64_t sum = 0;
    for (size_t c = 0; c < kChunks; ++c) {
        spert::CopyFuture next;
        if (c + 1 < kChunks) {
            // Queue the next chunk into the *other* buffer. This transfer runs
            // while the loop body below computes on the current one.
            next = ctx->copy_async(buffers[(c + 1) & 1u].data, ddr + (c + 1) * kChunkBytes, kChunkBytes);
        }

        sum += reduce_chunk(buffers[c & 1u].data, kChunkBytes);

        if (next.valid()) {
            next.sync();  // The next chunk is ready for the following iteration.
        }
    }

    *result = sum;
    ctx->free_shared(buf0);
    ctx->free_shared(buf1);
}

}  // namespace

int main() {
    // kMemMMZ is the engine-reachable kind. It returns null on backends without
    // device memory, which is the one and only availability check needed.
    uint8_t * ddr = static_cast<uint8_t *>(spert::Alloc(kChunks * kChunkBytes, spert::kMemMMZ));
    if (ddr == nullptr) {
        std::cout << "aidma_overlap: SKIP (no device memory on this backend)\n";
        return 0;
    }

    for (size_t c = 0; c < kChunks; ++c) {
        fill_chunk(ddr + c * kChunkBytes, c);
    }

    uint64_t result = 0;
    spert::Stream stream{ 1 };
    if (!stream.valid()) {
        std::cerr << "failed to create a SpineRuntime stream\n";
        spert::Free(ddr);
        return 1;
    }

    spert::Future future = stream.launch(spert::Grid{ 1 }, overlap_kernel, ddr, &result);
    if (!future.valid()) {
        std::cerr << "failed to launch the grid\n";
        spert::Free(ddr);
        return 2;
    }

    const spert::Status status = future.sync();
    if (status != spert::Status::Ok) {
        std::cerr << "grid failed: " << spert::to_string(status) << '\n';
        spert::Free(ddr);
        return 3;
    }

    uint64_t expected = 0;
    for (size_t c = 0; c < kChunks; ++c) {
        expected += expected_chunk(c);
    }
    if (result != expected) {
        std::cerr << "checksum mismatch: got " << result << ", expected " << expected << '\n';
        spert::Free(ddr);
        return 4;
    }

    std::cout << "aidma_overlap: " << kChunks << " chunks x " << kChunkBytes / 1024 << " KiB, checksum ok\n";
    spert::Free(ddr);
    return 0;
}
