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
    kDstOffsetAlign = 8,
};
const size_t kTotalSize = 1024ul * 1024 * 1024; // 1GB

TEST(ParisRuntimeTest, TestRawRead) {
    uint32_t *buffer =
        reinterpret_cast<uint32_t *>(aligned_alloc(kPageSize, kTotalSize));
    buffer[0] = kMagic;

    /* touch the buffer; the buffer[0] should be in remote after the touch */
    for (size_t i = kPageSize; i < kTotalSize; i += kPageSize) {
        *(reinterpret_cast<char *>(buffer) + i) = 0xf;
    }

    enum {
        kDstOffset = kDstOffsetAlign * 8,
    };

    runtime_init();
    uint32_t *object = reinterpret_cast<uint32_t *>(
        runtime_raw_read(buffer, sizeof(uint32_t), kDstOffset));

    ASSERT_NE(object, nullptr)
        << "The object is null, please check your cgroup memory limits";
    ASSERT_EQ((uintptr_t)object & (kPageSize - 1), kDstOffset);
    ASSERT_EQ(object[0], kMagic);

    runtime_exit();
    free(buffer);
}

TEST(ParisRuntimeTest, TestMultiRawRead) {
    enum {
        kNumThreads = 1023, // limited by the number of dma pages
        kNumThreadData = 32,
    };
    static_assert(kTotalSize / kPageSize >= kNumThreads * kNumThreadData);

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
                int align_offset = (sizeof(uint32_t) + kDstOffsetAlign - 1) /
                                   kDstOffsetAlign * kDstOffsetAlign;
                uint32_t *objects = reinterpret_cast<uint32_t *>(
                    runtime_raw_read(&buffer[id * kPageSize / sizeof(uint32_t)],
                                     sizeof(uint32_t), 0));
                ASSERT_NE(objects, nullptr)
                    << "The object is null, please "
                       "check your cgroup memory limits";
                ASSERT_EQ(*objects, kMagic + id);

                for (int j = 1; j < kNumThreadData; ++j) {
                    uint32_t *object =
                        reinterpret_cast<uint32_t *>(runtime_raw_read(
                            &buffer[(id + j) * kPageSize / sizeof(uint32_t)],
                            sizeof(uint32_t), align_offset * j));
                    ASSERT_EQ((uintptr_t)object,
                              (uintptr_t)objects + align_offset * j);
                    ASSERT_EQ(*object, kMagic + id + j);
                }
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