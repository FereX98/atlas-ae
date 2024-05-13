#pragma once
#include "allocator.h"
#include <asm/types.h>
#include <memory>

namespace paris {

class FileHandle {
  public:
    FileHandle(int fd) : fd_(fd) {}
    ~FileHandle();
    int Fd() { return fd_; }

  private:
    int fd_;
};

class ThreadLocalResource {
  public:
    ThreadLocalResource() : dma_handle_(0), mmap_page_(nullptr) {}
    ~ThreadLocalResource();
    __u64 Handle() { return dma_handle_; }
    void *Page() { return mmap_page_; }
    void Init(std::shared_ptr<FileHandle> fd, void *mmap_start);
    ThreadLocalAllocator &Allocator() { return allocator_; }

  private:
    ThreadLocalAllocator allocator_;
    std::shared_ptr<FileHandle> fd_;
    __u64 dma_handle_;
    void *mmap_page_;
};

class BksContext {
  public:
    BksContext();
    ~BksContext();
    void *Fetch(const void *object, int object_size);
    void *FetchTo(void *dst, const void *object, int object_size);
    bool FetchAsync(const void *object, int object_size, int *queue);
    void *Sync(int queue, int object_size);

    /* Read the object content without changing any reference.
     * This is useful for reading primitive array.
     */
    bool Read(void *dst, const void *object, int object_size);

    /* Low-level interface, eliminating memcpy
     */
    void *RawRead(const void *object, int object_size, int offset);

  private:
    static thread_local ThreadLocalResource resource_;
    bool IoctlFetch(const void *object, int object_size, int sync, int *queue,
                    int dst_offset);
    std::shared_ptr<FileHandle> fd_;
    void *mmap_pages_;
};

} // namespace paris