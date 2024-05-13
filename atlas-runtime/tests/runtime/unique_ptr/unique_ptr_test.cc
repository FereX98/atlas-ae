#include "pointer.h"
#include "runtime.h"
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

TEST(AtlasUniquePtrTest, TestRead) {
    const size_t kNumObjects = kTotalSize / sizeof(Object);
    std::vector<atlas::AtlasUniquePtr<Object>> objects(kNumObjects);
    Object *buffer =
        reinterpret_cast<Object *>(aligned_alloc(kPageSize, kTotalSize));

    memset(buffer, 0, kTotalSize);

    for (size_t i = 0; i < kNumObjects; i++) {
        objects[i].reset_t(buffer + i);
    }

    /* touch the buffer; the buffer[kNumObjects - 1] should be in remote after
     * the touch */
    for (long i = kNumObjects - 1; i >= 0; i--) {
        buffer[i].val[0] = kMagic + i;
    }

    printf("Initialized %lu objects\n", kNumObjects);

    runtime_init();
    /* set the psf to object-in manually */
    memset(global_psf, 0xf, BKS_PSF_MMAP_SIZE);

    Object *object_first = objects[0].deref_get();
    Object *object_last = objects[kNumObjects - 1].deref_get();

    ASSERT_EQ(object_first, buffer)
        << "The first object should not be evacuated";
    ASSERT_NE(object_last, buffer + kNumObjects - 1)
        << "The last object should be evacuated";
    ASSERT_EQ(object_last->val[0], kMagic + kNumObjects - 1);

    objects[0].deref_put(object_first);
    objects[kNumObjects - 1].deref_put(object_last);

    runtime_exit();
    free(buffer);
}

TEST(AtlasUniquePtrTest, TestSingleSync) {
    const size_t kNumObjects = kTotalSize / sizeof(Object);
    std::vector<atlas::AtlasUniquePtr<Object>> objects(kNumObjects);
    Object *buffer =
        reinterpret_cast<Object *>(aligned_alloc(kPageSize, kTotalSize));

    memset(buffer, 0, kTotalSize);

    for (size_t i = 0; i < kNumObjects; i++) {
        objects[i].reset_t(buffer + i);
    }

    runtime_init();
    /* set the psf to object-in manually */
    memset(global_psf, 0xf, BKS_PSF_MMAP_SIZE);

    buffer[0].val[0] = kMagic;
    /* hold the first object's raw ptr */
    Object *object_first = objects[0].deref_get();
    ASSERT_EQ(object_first, buffer);

    /* touch the buffer; the first object should be in remote after
     * the touch */
    for (size_t i = 0; i < kNumObjects; i++) {
        buffer[i].val[0] = kMagic + i;
    }
    printf("Initialized %lu objects\n", kNumObjects);

    /* another deref of the first object, as someone holds the raw ptr, it
     * cannot be evacuated */
    Object *object_first_new = objects[0].deref_get();

    ASSERT_EQ(object_first_new, object_first) << "The new deref should be the "
                                                 "same as the old one";
    objects[0].deref_put(object_first);
    objects[0].deref_put(object_first_new);

    /* deref again, the object should be evacuated as no one holds the raw ptr
     */
    object_first = objects[0].deref_get();

    ASSERT_NE(object_first, buffer) << "The object should be evacuated";

    ASSERT_EQ(object_first->val[0], kMagic);

    objects[0].deref_put(object_first);

    runtime_exit();
    free(buffer);
}

TEST(AtlasUniquePtrTest, TestMultiSync) {
    const size_t kNumObjects = kTotalSize / sizeof(Object);
    std::vector<atlas::AtlasUniquePtr<Object>> objects(kNumObjects);
    Object *buffer =
        reinterpret_cast<Object *>(aligned_alloc(kPageSize, kTotalSize));

    memset(buffer, 0, kTotalSize);

    for (size_t i = 0; i < kNumObjects; i++) {
        objects[i].reset_t(buffer + i);
    }

    runtime_init();
    /* set the psf to object-in manually */
    memset(global_psf, 0xf, BKS_PSF_MMAP_SIZE);

    buffer[0].val[0] = kMagic;
    /* hold the first object's raw ptr */
    Object *object_first = objects[0].deref_get();
    Object *raw_ptr = &buffer[0];
    ASSERT_EQ(object_first, raw_ptr);

    /* touch the buffer; the first object should be in remote after
     * the touch */
    for (size_t i = 0; i < kNumObjects; i++) {
        buffer[i].val[0] = kMagic + i;
    }
    printf("Initialized %lu objects\n", kNumObjects);

    enum {
        kNumThreads = 128,
    };

    std::vector<std::thread> threads;
    /* create multiple threads to deref the first object concurrently, someone
     * will finally evacuate the object. */
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&objects, raw_ptr]() {
            while (true) {
                Object *object = objects[0].deref_get();
                cpu_relax();
                objects[0].deref_put(object);
                if (object == raw_ptr) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    cpu_relax();
                } else {
                    break;
                }
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    /* release main thread's raw ptr to enable evacuation */
    objects[0].deref_put(object_first);

    for (int i = 0; i < kNumThreads; ++i) {
        threads[i].join();
    }

    object_first = objects[0].deref_get();

    ASSERT_NE(object_first, buffer) << "The object should have been evacuated";

    ASSERT_EQ(object_first->val[0], kMagic);

    objects[0].deref_put(object_first);

    runtime_exit();
    free(buffer);
}

static unsigned get_psf_index(uintptr_t ptr) {
    BARRIER_ASSERT(ptr <= BKS_PSF_VA_BASE &&
                   ptr > BKS_PSF_VA_BASE - BKS_PSF_MAX_MEM_MB * 1024 * 1024);
    unsigned psf_index = (BKS_PSF_VA_BASE - ptr) >> BKS_PSF_CHUNK_SHIFT;
    return psf_index;
}

TEST(AtlasUniquePtrTest, TestMultiPsfSync) {
    const size_t kNumObjects = kTotalSize / sizeof(Object);
    std::vector<atlas::AtlasUniquePtr<Object>> objects(kNumObjects);
    Object *buffer =
        reinterpret_cast<Object *>(aligned_alloc(kPageSize, kTotalSize));

    memset(buffer, 0, kTotalSize);

    for (size_t i = 0; i < kNumObjects; i++) {
        objects[i].reset_t(buffer + i);
    }

    runtime_init();

    unsigned start_psf = get_psf_index((uintptr_t)buffer);
    unsigned num_psf = kTotalSize >> BKS_PSF_CHUNK_SHIFT;

    memset(global_psf, 0x1, BKS_PSF_MMAP_SIZE);

    /* touch the buffer; the first object should be in remote after
     * the touch */
    for (size_t i = 0; i < kNumObjects; i++) {
        buffer[i].val[0] = kMagic + i;
    }
    printf("Initialized %lu objects\n", kNumObjects);

    /* used to control threads, 1 to start all threads, 2 to stop psf thread */
    std::atomic_int flag(0);

    enum {
        kNumThreads = 256,
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&objects, &flag, buffer]() {
            while (flag.load() == 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }

            size_t cnt = 0;
            enum {
                kLoopCnt = 100000,
            };

            while (cnt < kLoopCnt) {
                unsigned obj_idx = rand() % kNumObjects;
                Object *object = objects[obj_idx].deref_get();
                cpu_relax();
                ASSERT_EQ(object->val[0], kMagic + obj_idx);
                objects[obj_idx].deref_put(object);
                if (object == buffer + obj_idx) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
                cnt++;
            }
        });
    }

    /* create the psf thread to randomly flip psf */
    threads.emplace_back([start_psf, num_psf, &flag]() {
        while (flag.load() == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        while (flag.load() == 1) {
            unsigned psf_idx = start_psf + (rand() % num_psf);
            /* flip the psf */
            std::atomic_store(
                (std::atomic_uint8_t *)&global_psf[psf_idx].psf,
                1 - std::atomic_load(
                        (std::atomic_uint8_t *)&global_psf[psf_idx].psf));

            cpu_relax();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    flag.store(1);

    for (int i = 0; i < kNumThreads; ++i) {
        threads[i].join();
    }
    flag.store(2);
    threads[kNumThreads].join();

    size_t evacuated_objects = 0;
    for (size_t i = 0; i < kNumObjects; i++) {
        uint64_t orig_val = *(volatile uint64_t *)&buffer[i].val[0];
        /* make sure the object is in local */
        asm volatile("" ::: "memory");
        Object *object = objects[i].deref_get();
        if (object != buffer + i)
            evacuated_objects++;
        ASSERT_EQ(object->val[0], orig_val);
    }
    printf("Evacuate %lu objects of %lu objects\n", evacuated_objects,
           kNumObjects);

    runtime_exit();
    free(buffer);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
