#include "allocator.h"
#include <cstdlib>
#include <memory>

namespace paris {

enum {
    kMempoolPages = 7864320,  // 30GB
    kThreadLocalPages = 512, // 2MB
};

GlobalAllocator &GlobalAllocator::GetInstance() {
    static GlobalAllocator instance;
    return instance;
}

GlobalAllocator::GlobalAllocator() {
    // mempool_ = aligned_alloc(4096, kMempoolPages * 4096ull);
    // BARRIER_ASSERT(mempool_ != nullptr);
    // mempool_end_ = (char *)mempool_ + kMempoolPages * 4096ull;
    // total_pages_ = kMempoolPages;
    // next_page_ = 0;
}

GlobalAllocator::~GlobalAllocator() { free(mempool_); }

void *GlobalAllocator::Alloc(int pages) {
    // BARRIER_ASSERT(pages > 0 && pages <= total_pages_ - next_page_);
    // int page_index = next_page_.fetch_add(pages);
    // void *ret = (char *)mempool_ + page_index * 4096ull;
    // return ret;
    return nullptr;
}

void GlobalAllocator::Free(void *ptr, int pages) {
    // not implemented
    BARRIER_ASSERT(false);
}

ThreadLocalAllocator::ThreadLocalAllocator()
    : mem_(nullptr), next_(0), total_(0) {
    global_allocator_ = &GlobalAllocator::GetInstance();
}

ThreadLocalAllocator::~ThreadLocalAllocator() {}

void *ThreadLocalAllocator::Alloc(size_t size) {
    // // huge allocation for large than 1/4 kThreadLocalPages
    // if (size > 4096ull * kThreadLocalPages / 4) {
    //     return global_allocator_->Alloc(ALIGN_UP(size, 4096ull) / 4096ull);
    // }

    // if (next_ + size > total_) {
    //     mem_ = (char *)global_allocator_->Alloc(kThreadLocalPages);
    //     next_ = 0;
    //     total_ = kThreadLocalPages * 4096ull;
    // }

    // void *ret = mem_ + next_;
    // next_ += ALIGN_UP(size, 8);
    // return ret;
    // return atlas_malloc(size);
    return malloc(size);
}

void ThreadLocalAllocator::Free(void *ptr, size_t size) {
    // not implemented
    // BARRIER_ASSERT(false);
}

} // namespace paris
