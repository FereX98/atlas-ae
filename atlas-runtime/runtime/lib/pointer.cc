#include "pointer.h"
#include "allocator.h"
#include "bks_ctx.h"
#include "pointer_shim.h"
#include "tsx.h"
#include <cstring>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <unistd.h>



extern paris::BksContext *bks_ctx;

std::atomic<unsigned long> num_fetches;
unsigned long last_num_fetches;

namespace atlas {

template <bool card_prof>
static FORCE_INLINE void *paging_in(AtlasPtrMeta &meta) {
    meta.inc_refcnt<card_prof>();
    return (void *)meta.get_object_addr();
}

/* atomically clear the evacuation bit and either update object address (if
 * evacuation succ) or increase refcnt (if fail)
 * return the final object address
 */
FORCE_INLINE void *AtlasPtrMeta::finish_evacuation(uint64_t new_object_addr) {
    meta_t old_meta = {.u64 = load()};
    BARRIER_ASSERT(old_meta.s.evacuation && old_meta.s.refcnt == 0 &&
                   old_meta.s.tospace == 0);
    meta_t new_meta = {.u64 = old_meta.u64};
    new_meta.s.evacuation = 0;
    void *object_ptr = nullptr;
    if (new_object_addr != 0) {
        // Shi: count the number of successful object fetches here
        ++num_fetches;
        // If disabling to-space, uncomment refcnt assignment
        new_meta.s.tospace = 1;
        //new_meta.s.refcnt = 1;
        new_meta.s.obj_addr = new_object_addr;
        object_ptr = (void *)new_object_addr;
    } else {
        new_meta.s.refcnt = 1;
        object_ptr = AtlasPtrMeta::to_ptr(old_meta);
    }

    if (!metadata_.au64.compare_exchange_strong(old_meta.u64, new_meta.u64)) {
        BARRIER_ASSERT(
            false &&
            " someone changed the metadata when the object is evacuating");
    }
    // Shi: This leads to immediate psf flip
    //card_profiling();

    return object_ptr;
}

void AtlasPtrMeta::update_metadata(uint64_t object_addr, unsigned object_size) {
    do {
        uint64_t old = load();
        if (old & kEvacuationBitMask) {
            cpu_relax();
            continue;
        }
        auto skip_check = [](const meta_t &meta) -> bool {
            return meta.s.obj_size == 0 ||
                   ((meta.s.obj_addr & PAGE_MASK) + get_size(meta) > PAGE_SIZE);
        };

        meta_t old_meta{
            .u64 = old,
        };
        meta_t new_meta{
            .u64 = AtlasPtrMeta(0, object_addr, object_size).metadata_.u64,
        };

        /* It should be careful when update object's addr and size to make sure
         * the refcnt is updated correctly in deref_get & deref_put */
        BARRIER_ASSERT(skip_check(old_meta) == skip_check(new_meta));

        old_meta.s.obj_addr = new_meta.s.obj_addr;
        old_meta.s.obj_size = new_meta.s.obj_size;
        if (metadata_.au64.compare_exchange_strong(old, old_meta.u64)) {
            return;
        };
        cpu_relax();
    } while (true);
}

template <bool card_prof>
void *AtlasGenericPtr::deref_get_slow_path(uint64_t object_addr) {
    bool remote = tsx_remote_check((void *)object_addr);
    if (!remote) {
        return paging_in<card_prof>(meta_);
    }

    bool succ = meta_.try_set_evacuation();
    /* cannot evacuate the object, either because some threads are holding the
     * page refcnt, or the object is being evacuated */
    if (!succ) {
        return paging_in<card_prof>(meta_);
    }
    /* [?]: the address may change ? */
    object_addr = meta_.get_object_addr();

    BARRIER_ASSERT(bks_ctx != nullptr);

    void *new_object_addr =
        bks_ctx->Fetch((void *)object_addr, meta_.get_object_size());
    return meta_.finish_evacuation((uint64_t)new_object_addr);
}

void *AtlasGenericPtr::deref_no_scope_slow_path(uint64_t object_addr) {
    bool remote = tsx_remote_check((void *)object_addr);
    if (!remote) {
        return (void *)object_addr;
    }

    BARRIER_ASSERT(bks_ctx != nullptr);

    void *new_object_addr =
        bks_ctx->Fetch((void *)object_addr, meta_.get_object_size());
    if (new_object_addr) {
        meta_.set_object_addr((uint64_t)new_object_addr);
        // If disabling to-space, uncomment refcnt assignment
        meta_.metadata_.s.tospace = 1;
        //meta_.metadata_.s.refcnt = 1;
    } else {
        new_object_addr = (void *)object_addr;
    }
    return new_object_addr;
}

void AtlasGenericPtr::deref_put_slow_path(void *ptr) { meta_.dec_refcnt(); }

void *AtlasGenericPtr::deref_raw_slow_path(uint64_t object_addr, int offset) {
    bool remote = tsx_remote_check((void *)object_addr);
    if (!remote) {
        return (void *)object_addr;
    }

    BARRIER_ASSERT(bks_ctx != nullptr);

    void *new_object_addr =
        bks_ctx->RawRead((void *)object_addr, meta_.get_object_size(), offset);

    if (!new_object_addr) {
        new_object_addr = (void *)object_addr;
    }
    return new_object_addr;
}

void *AtlasGenericPtr::deref_readonly_slow_path(uint64_t object_addr,
                                                void *dst) {
    bool remote = tsx_remote_check((void *)object_addr);
    if (!remote) {
        return (void *)object_addr;
    }

    BARRIER_ASSERT(bks_ctx != nullptr);

    bool r = bks_ctx->Read(dst, (void *)object_addr, meta_.get_object_size());

    return r ? dst : (void *)object_addr;
}

void atlas_free_object(void *ptr) {}

} // namespace atlas

static_assert(sizeof(atlas_unique_ptr) == sizeof(atlas::AtlasUniquePtr<void>));

atlas_unique_ptr atlas_make_unique_ptr(void *object, unsigned object_size) {
    atlas_unique_ptr up;
    new (&up) atlas::AtlasUniquePtr<void>((uintptr_t)object, object_size);
    return up;
}

void *atlas_up_deref_get(atlas_unique_ptr *up) {
    return ((atlas::AtlasUniquePtr<void> *)up)->deref_get();
}

void *atlas_up_deref_get_no_card(atlas_unique_ptr *up) {
    return ((atlas::AtlasUniquePtr<void> *)up)->deref_get_no_card();
}

void *atlas_up_force_move(atlas_unique_ptr *up) {
    size_t object_size = ((atlas::AtlasUniquePtr<void> *)up)->get_size();
    enum {
        kAddrShift = 17,
        kThres = 100,
    };

    void *object = (void *)(up->handle >> kAddrShift);
    if (!object_size) {
        return object;
    }

    static int remote_cnt = 0;
    if (remote_cnt < kThres && tsx_remote_check(object)) {
        void *new_object = bks_ctx->Fetch(object, object_size);
        if (new_object) {
            remote_cnt++;
            return new_object;
        }
    } else if (remote_cnt >= kThres) {
        void *new_object = malloc(object_size);
        memcpy(new_object, object, object_size);
        return new_object;
    }
    return object;
}

void *atlas_up_deref_no_scope(atlas_unique_ptr *up) {
    return ((atlas::AtlasUniquePtr<void> *)up)->deref_no_scope();
}

void *atlas_up_deref_get_paging(atlas_unique_ptr *up) {
    return ((atlas::AtlasUniquePtr<void> *)up)->deref_get_paging();
}

void *atlas_up_deref_get_evicted(atlas_unique_ptr *up, bool *evicted) {
    void *res = ((atlas::AtlasUniquePtr<void> *)up)->deref_get();
    *evicted = ((atlas::AtlasUniquePtr<void> *)up)->is_tospace();
    return res;
}

void atlas_up_deref_put(atlas_unique_ptr *up, void *ptr) {
    ((atlas::AtlasUniquePtr<void> *)up)->deref_put(ptr);
}

void *atlas_up_deref_raw(atlas_unique_ptr *up, int offset) {
    return ((atlas::AtlasUniquePtr<void> *)up)->deref_raw(offset);
}

void atlas_up_reset(atlas_unique_ptr *up, void *ptr, unsigned object_size) {
    ((atlas::AtlasUniquePtr<void> *)up)->reset(ptr, object_size);
}

void atlas_up_release(atlas_unique_ptr *up) {
    ((atlas::AtlasUniquePtr<void> *)up)->~AtlasUniquePtr();
}

void atlas_up_mark_evacuation(atlas_unique_ptr *up) {
    ((atlas::AtlasUniquePtr<void> *)up)->mark_evacuation();
}

void atlas_up_clear_evacuation(atlas_unique_ptr *up) {
    ((atlas::AtlasUniquePtr<void> *)up)->clear_evacuation();
}

/*
 * Ideally, we can use template args to calculate the header size. However, for
 * redis, it could not know exactly what the header type is until we examine the
 * last byte of the header (i.e. the byte before object data). There are two
 * solutions:
 *  1. use an extra field to record the header size (non-negligible space
 * overhead)
 *  2. use the largest header type (RDMA data amplification and to-space
 * space overhead) // PS: the to-space overhead can be eliminated if we check
 *      the header when copy the object from dma buffer to to-space.
 *  We choose the second one for now.
 *
 */
typedef uint64_t redis_sds_type;

atlas_unique_ptr_wh atlas_make_unique_ptr_wh(void *object, unsigned object_size,
                                             unsigned header_size) {
    atlas_unique_ptr_wh up;
    /* we don't expect the redis to store strings larger than sdshdr16 type */
    BARRIER_ASSERT(header_size <= sizeof(redis_sds_type));
    new (&up) atlas::AtlasUniquePtrWithHeader<void, redis_sds_type>(
        object, object_size);
    return up;
}

void *atlas_upwh_deref_get(atlas_unique_ptr_wh *up) {
    return ((atlas::AtlasUniquePtrWithHeader<void, redis_sds_type> *)up)
        ->deref_get();
}

void atlas_upwh_deref_put(atlas_unique_ptr_wh *up, void *ptr) {
    ((atlas::AtlasUniquePtrWithHeader<void, redis_sds_type> *)up)
        ->deref_put(ptr);
}

void *atlas_upwh_deref_raw(atlas_unique_ptr_wh *up, int offset) {
    return ((atlas::AtlasUniquePtrWithHeader<void, redis_sds_type> *)up)
        ->deref_raw(offset);
}

void atlas_upwh_reset(atlas_unique_ptr_wh *up, void *ptr, unsigned object_size,
                      unsigned header_size) {
    BARRIER_ASSERT(header_size <= sizeof(redis_sds_type));
    ((atlas::AtlasUniquePtrWithHeader<void, redis_sds_type> *)up)
        ->reset(ptr, object_size);
}

void atlas_upwh_release(atlas_unique_ptr_wh *up) {
    ((atlas::AtlasUniquePtrWithHeader<void, redis_sds_type> *)up)
        ->~AtlasUniquePtrWithHeader();
}

void atlas_fetch_to(char *dst, char *src, size_t size) {
    const int page_size = 4096;
    BARRIER_ASSERT(size == 2 * page_size);
    BARRIER_ASSERT(((uint64_t)src) % page_size == 0);
    BARRIER_ASSERT(bks_ctx != nullptr);

    // First page
    bool remote = tsx_remote_check(src);
    if (!remote) {
        memcpy(dst, src, page_size);
    }
    void *new_object_addr = bks_ctx->FetchTo(dst, src, page_size);
    if (new_object_addr == 0) {
        memcpy(dst, src, page_size);
    }

    // Second page
    dst += page_size;
    src += page_size;
    remote = tsx_remote_check(src);
    if (!remote) {
        memcpy(dst, src, page_size);
    }
    new_object_addr = bks_ctx->FetchTo(dst, src, page_size);
    if (new_object_addr == 0) {
        memcpy(dst, src, page_size);
    }

}