#include "runtime.h"
#include <chrono>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>

enum {
    kPageSize = 4096,
    kMagic = 0xdeadbeefu,
};
const size_t kTotalSize = 1024ul * 1024 * 1024; // 1GB

TEST(ParisRuntimeTest, TestObjectFetch) {
    uint32_t *buffer =
        reinterpret_cast<uint32_t *>(aligned_alloc(kPageSize, kTotalSize));
    buffer[0] = kMagic;

    /* touch the buffer; the buffer[0] should be in remote after the touch */
    for (size_t i = kPageSize; i < kTotalSize; i += kPageSize) {
        *(reinterpret_cast<char *>(buffer) + i) = 0xf;
    }

    runtime_init();
    uint32_t *object =
        reinterpret_cast<uint32_t *>(runtime_fetch(buffer, sizeof(uint32_t)));

    ASSERT_NE(object, nullptr)
        << "The object is null, please check your cgroup memory limits";
    ASSERT_EQ(object[0], kMagic);

    runtime_exit();
    free(buffer);
}

TEST(ParisRuntimeTest, TestMultiFetch) {
    enum {
        kNumThreads = 1023, // limited by the number of dma pages
    };

    uint32_t *buffer =
        reinterpret_cast<uint32_t *>(aligned_alloc(kPageSize, kTotalSize));
    // init each page using the magic number plus page id
    for (size_t i = 0; i < kTotalSize; i += kPageSize) {
        buffer[i / sizeof(uint32_t)] = kMagic + i / kPageSize;
    }

    runtime_init();
    std::vector<std::thread> threads;

    for (int i = 0; i < kNumThreads; i++) {
        threads.emplace_back(std::thread(
            [buffer](int id) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                uint32_t *object = reinterpret_cast<uint32_t *>(
                    runtime_fetch(&buffer[id * kPageSize / sizeof(uint32_t)],
                                  sizeof(uint32_t)));
                ASSERT_NE(object, nullptr) << "The object is null, please "
                                              "check your cgroup memory limits";
                ASSERT_EQ(*object, kMagic + id);
            },
            i));
    }

    for (int i = 0; i < kNumThreads; i++) {
        threads[i].join();
    }

    runtime_exit();
    free(buffer);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}