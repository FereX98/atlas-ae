#include "pointer.h"
#include "runtime.h"
#include "zipf.h"
#include <bks_types.h>
#include <chrono>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>

enum {
    kPageSize = 4096,
    kMagic = 0xdeadbeefu,
    kObjectWords = 16,
};
const size_t kTotalSize = 1024ul * 1024 * 1024; // 1GB

struct Object {
    uint64_t val[kObjectWords];
};

TEST(AtlasArrayTest, TestZipfAccess) {
    const size_t kNumObjects = kTotalSize / sizeof(Object);
    std::vector<atlas::AtlasUniquePtr<Object>> objects(kNumObjects);
    Object *buffer =
        reinterpret_cast<Object *>(aligned_alloc(kPageSize, kTotalSize));

    memset(buffer, 0, kTotalSize);

    for (size_t i = 0; i < kNumObjects; i++) {
        objects[i].reset_t(buffer + i);
    }

    for (size_t i = 0; i < kNumObjects; i++) {
        buffer[i].val[0] = kMagic + i;
    }

    const size_t kNumThreads = 128;
    const size_t kNumRequests = kNumObjects;
    const double kZipfParamS = 0.9;

    printf("Initialized %lu objects\n", kNumObjects);

    runtime_init();
    memset(global_psf, 0xf, BKS_PSF_MMAP_SIZE);

    std::vector<uint64_t> requests(kNumRequests);
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        far_memory::zipf_table_distribution<> zipf(kNumObjects, kZipfParamS);
        for (uint64_t i = 0; i < kNumRequests; ++i) {
            requests[i] = zipf(gen);
        }
        printf("Generate %lu requests\n", kNumRequests);
    }

    char cmd[256];
    memset(cmd, 0, sizeof(cmd));
    /* 50% local memory */
    uint64_t limit_mb =
        (kTotalSize + kNumRequests * sizeof(uint64_t)) / 1024 / 1024 / 2;
    sprintf(cmd, "cgset -r memory.limit_in_bytes=%ldM test", limit_mb);
    int r = system(cmd);
    printf("Set the local memory limit %ldM, error code: %d\n", limit_mb, r);

    std::vector<std::thread> threads;
    std::vector<uint64_t> sums(kNumThreads);
    for (size_t t = 0; t < kNumThreads; ++t) {
        threads.emplace_back([&objects, &requests, t, &sums]() {
            uint64_t sum = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            for (uint64_t i = 0; i < kNumRequests; ++i) {
                Object *object = objects[requests[i]].deref_get();
                cpu_relax();
                sum += object->val[0];
                objects[requests[i]].deref_put(object);
                if (t == 0 && i % 1000000 == 0)
                    printf("Thread 0: %lu requests done\n", i);
            }
            sums[t] = sum;
        });
    }

    for (uint64_t t = 0; t < kNumThreads; ++t) {
        threads[t].join();
    }

    for (uint64_t t = 1; t < kNumThreads; ++t) {
        ASSERT_EQ(sums[t], sums[0]);
    }
    printf("Final result is 0x%lx\n", sums[0]);

    runtime_exit();
    free(buffer);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
