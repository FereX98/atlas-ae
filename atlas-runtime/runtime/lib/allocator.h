#pragma once
#include "helpers.h"
#include <atomic>
#include <cstddef>
#include <jemalloc/jemalloc.h>

namespace paris {

// class ThreadLocalAllocator {
//   public:
//     /* TODO: impl our log-structured allocator
//      * use jemalloc as the to-space allocator for now
//      */
//     static void *Alloc(size_t size) { return atlas_malloc(size); }
//     static void Free(void *ptr) { atlas_free(ptr); }
// };
class GlobalAllocator {
  public:
    static GlobalAllocator &GetInstance();
    void *Alloc(int pages);
    void Free(void *ptr, int pages);
    bool IsIn(const void *ptr) {
        if ((uintptr_t)ptr >= (uintptr_t)mempool_ &&
            (uintptr_t)ptr < (uintptr_t)mempool_end_) {
            return true;
        }
        return false;
    }

  private:
    GlobalAllocator();
    ~GlobalAllocator();
    void *mempool_;
    void *mempool_end_;
    std::atomic_int32_t next_page_;
    int total_pages_;
};

class ThreadLocalAllocator {
  public:
    ThreadLocalAllocator();
    ~ThreadLocalAllocator();
    void *Alloc(size_t size);
    void Free(void *ptr, size_t size);

  private:
    GlobalAllocator *global_allocator_;
    char *mem_;
    size_t next_;
    size_t total_;
};

} // namespace paris