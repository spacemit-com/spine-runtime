#include "spert.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

void fill_squares(spert::Context* ctx, uint32_t* output, uint32_t count) {
    const uint32_t index = ctx->program_id(0);
    if (index < count) {
        output[index] = index * index;
    }
}

int main() {
    constexpr uint32_t kTiles = 8;
    std::vector<uint32_t> output(kTiles, 0);

    spert::Stream stream;
    if (!stream.valid()) {
        std::cerr << "failed to create a SpineRuntime stream\n";
        return 1;
    }

    spert::Future future =
        stream.launch(spert::Grid(kTiles), fill_squares, output.data(), kTiles);
    if (!future.valid()) {
        std::cerr << "failed to launch the grid\n";
        return 2;
    }

    const spert::Status status = future.sync();
    if (status != spert::Status::Ok) {
        std::cerr << "grid failed: " << spert::to_string(status) << '\n';
        return 3;
    }

    for (uint32_t i = 0; i < kTiles; ++i) {
        if (output[i] != i * i) {
            std::cerr << "unexpected result at tile " << i << '\n';
            return 4;
        }
    }

    std::cout << "SpineRuntime quickstart passed on " << stream.core_count()
              << " core(s)\n";
    return 0;
}
