#pragma once
#include "helpers.h"
#include <atomic>
#include <bks_types.h>
#include <stdint.h>
#include <stdio.h>
#include <thread>

extern struct bks_psf *global_psf;
extern struct bks_card *global_card;
extern bool card_profiling_on;

namespace atlas {
template <typename T> class AtlasUniquePtr;
template <typename T, typename H> class AtlasUniquePtrWithHeader;
// Format:
//  | U (1b) | R (6b) | T (1b) | E(1b) | Obj Size (8b) | Obj Addr(47b)  |
//                  U: unused bit.
//                  R: refcnt bits.
//                  T: to-space bit, indicates the object is in to-space.
//                  E: being evacuation.
//           Obj Size: the size (in 4 bytes) of the pointed object, 0 for
//           objects
//                       which are large enough to page-in.
//           Obj Addr: the address of the referenced object's data.

class AtlasPtrMeta {
  private:
    constexpr static uint64_t kWordSize = sizeof(unsigned);
    constexpr static uint64_t kUnusedBits = 1;
    constexpr static uint64_t kRefcntBitPos = kUnusedBits;
    constexpr static uint64_t kRefcntBits = 5;
    constexpr static uint64_t kRefcntMax = (1ul << kRefcntBits) - 1;
    constexpr static uint64_t kRefcntMask = kRefcntMax << kRefcntBitPos;
    constexpr static uint64_t kTospaceBitPos = kRefcntBitPos + kRefcntBits;
    constexpr static uint64_t kTospaceMask = 1ul << kTospaceBitPos;
    constexpr static uint64_t kEvacuationBitPos = kTospaceBitPos + 1;
    constexpr static uint64_t kEvacuationBitMask = 1 << kEvacuationBitPos;
    constexpr static uint64_t kObjectSizeBitPos = kEvacuationBitPos + 1;
    constexpr static uint64_t kObjectSizeBits = 9;
    constexpr static uint64_t kMaxObjectSize = (1 << kObjectSizeBits) - 1;
    constexpr static uint64_t kObjectAddrBits = 47;

    static_assert(kObjectSizeBitPos + kObjectSizeBits + kObjectAddrBits == 64,
                  "The metadata is not 64 bits");

    union meta_t {
        uint64_t u64;
        std::atomic_uint64_t au64;
        struct {
            unsigned long unused : kUnusedBits;
            unsigned long refcnt : kRefcntBits;
            unsigned long tospace : 1;
            unsigned long evacuation : 1;
            unsigned long obj_size : kObjectSizeBits;
            unsigned long obj_addr : kObjectAddrBits;
        } s;
    } metadata_;

    static_assert(sizeof(metadata_) == sizeof(void *),
                  "AtlasPtrMeta size mismatch");
    friend class AtlasGenericPtr;
    template <typename T> friend class AtlasUniquePtr;
    template <typename T, typename H> friend class AtlasUniquePtrWithHeader;

    FORCE_INLINE AtlasPtrMeta() { metadata_.u64 = 0; }

    void init(uint64_t object_addr, unsigned object_size) {

        unsigned words = DIV_UP(object_size, kWordSize);
        if (words > kMaxObjectSize) {
            words = 0;
        }
        meta_t meta = {
            .u64 = 0,
        };
        meta.s.obj_addr = object_addr;
        meta.s.obj_size = words;
        metadata_.u64 = meta.u64;
    }

  public:
    AtlasPtrMeta(bool tospace, uint64_t object_addr, unsigned object_size) {
        init(object_addr, object_size);
        metadata_.s.tospace = tospace;
    }

    FORCE_INLINE bool is_tospace() const { return metadata_.s.tospace; }

    FORCE_INLINE bool try_set_evacuation() {
        do {
            uint64_t old_meta = load();
            if ((old_meta & kTospaceMask) || (old_meta & kRefcntMask) ||
                (old_meta & kEvacuationBitMask)) {
                return false;
            }
            if (metadata_.au64.compare_exchange_strong(
                    old_meta, old_meta | kEvacuationBitMask)) {
                return true;
            };
            cpu_relax();
        } while (true);
    }

    FORCE_INLINE void mark_evacuation() {
        do {
            uint64_t old_meta = load();
            if ((old_meta & kRefcntMask) || (old_meta & kEvacuationBitMask)) {
                cpu_relax();
                continue;
            }
            if (metadata_.au64.compare_exchange_strong(
                    old_meta, old_meta | kEvacuationBitMask)) {
                return;
            };
            cpu_relax();
        } while (true);
    }

    FORCE_INLINE void clear_evacuation() {
        BARRIER_ASSERT(is_evacuation());
        metadata_.s.evacuation = 0;
    }

    void *finish_evacuation(uint64_t new_object_addr);

    FORCE_INLINE bool is_evacuation() const {
        return !!(metadata_.au64.load(std::memory_order_acquire) &
                  kEvacuationBitMask);
    }

    FORCE_INLINE void card_profiling() {
        uint64_t addr = get_object_addr();
        uint16_t size = get_object_size();
        uint64_t addr_last_byte = addr + size - 1;
        const uint64_t card_bit_size = sizeof(struct bks_card) * 8;
        BARRIER_ASSERT(card_bit_size == 32);
        BARRIER_ASSERT(BKS_PSF_VA_BASE > addr);
        // Find the page card the object corresponds to
        uint64_t page_idx = (BKS_PSF_VA_BASE - addr) >> BKS_PAGE_SHIFT;
        uint64_t page_offset = addr % BKS_PAGE_SIZE;
        uint64_t first_card_idx = page_offset >> BKS_CARD_SHIFT;
        uint64_t last_card_idx;
        // TODO: restriction here: we only consider cards on the first page
        if (page_offset + size >= BKS_PAGE_SIZE) {
            last_card_idx = card_bit_size - 1;
        } else {
            last_card_idx = (addr_last_byte % BKS_PAGE_SIZE) >> BKS_CARD_SHIFT;
        }
        BARRIER_ASSERT(first_card_idx <= last_card_idx);
        BARRIER_ASSERT(last_card_idx < card_bit_size);
        uint64_t num_cards = last_card_idx - first_card_idx + 1;
        uint32_t mask = 0xFFFFFFFF >> (card_bit_size-num_cards);     
        std::atomic_uint32_t *old_card_p = ((std::atomic_uint32_t *)global_card) + page_idx;
        uint32_t old_card = old_card_p->load();
        uint32_t new_card = old_card | (mask << first_card_idx);
        while (!(old_card_p->compare_exchange_strong(old_card, new_card))) {
            cpu_relax();
        }
    }

    template <bool card_prof>
    FORCE_INLINE void inc_refcnt() {
        do {
            uint64_t old_meta = load();
            uint64_t refcnt = (old_meta & kRefcntMask) >> kRefcntBitPos;

            if (old_meta & kTospaceMask) {
                return;
            }

            /* the refcnt is full, wait until someone release it */
            if (refcnt == kRefcntMax) {
                std::this_thread::yield();
                cpu_relax();
                continue;
            }

            /* the object is in evacuation */
            if (old_meta & kEvacuationBitMask) {
                cpu_relax();
                continue;
            }

            meta_t new_meta = {
                .u64 = old_meta,
            };
            new_meta.s.refcnt = refcnt + 1;

            if (metadata_.au64.compare_exchange_strong(old_meta,
                                                       new_meta.u64)) {
                // The object is accessed via paging
                // set cards
                if (card_prof && card_profiling_on) {
                    card_profiling();
                }
                return;
            };
            cpu_relax();
        } while (true);
    }

    FORCE_INLINE void dec_refcnt() {
        do {
            uint64_t old_meta = load();
            if (old_meta & kTospaceMask) {
                return;
            }

            uint64_t refcnt = (old_meta & kRefcntMask) >> kRefcntBitPos;

            BARRIER_ASSERT(refcnt > 0);

            meta_t new_meta = {
                .u64 = old_meta,
            };
            new_meta.s.refcnt = refcnt - 1;

            if (metadata_.au64.compare_exchange_strong(old_meta,
                                                       new_meta.u64)) {
                return;
            };
            cpu_relax();
        } while (true);
    }

    uint64_t get_object_addr() const { return metadata_.s.obj_addr; }

    void set_object_addr(uint64_t object_addr) {
        metadata_.s.obj_addr = object_addr;
    }

    uint16_t get_object_size() const {
        return metadata_.s.obj_size * kWordSize;
    };

    bool is_null() const { return metadata_.u64 == 0; };

    void nullify() { metadata_.u64 = 0; };
    FORCE_INLINE uint64_t load() {
        return metadata_.au64.load(std::memory_order_acquire);
    }
    static FORCE_INLINE void *to_ptr(const meta_t &meta) {
        return reinterpret_cast<void *>((unsigned long)meta.s.obj_addr);
    }
    static FORCE_INLINE uint16_t get_size(const meta_t &meta) {
        return meta.s.obj_size * kWordSize;
    }
    void update_metadata(uint64_t object_addr, unsigned object_size);
};

class AtlasGenericPtr {
  protected:
    AtlasPtrMeta meta_;
    template <bool card_prof> void *deref_get_slow_path(uint64_t object_addr);
    void *deref_no_scope_slow_path(uint64_t object_addr);
    void deref_put_slow_path(void *ptr);
    void *deref_raw_slow_path(uint64_t object_addr, int offset);
    void *deref_readonly_slow_path(uint64_t object_addr, void *dst);

    AtlasGenericPtr() {}
    AtlasGenericPtr(uint64_t object_addr, unsigned object_size) {
        meta_.init(object_addr, object_size);
    }
    FORCE_INLINE AtlasPtrMeta &meta() { return meta_; }
    FORCE_INLINE const AtlasPtrMeta &meta() const { return meta_; }
    template <bool kRefcnt, bool card_prof>
    FORCE_INLINE bool should_paging(const AtlasPtrMeta::meta_t &meta) {
        /* skip to-space objects and cross-page objects for now */
        // Shi: Allow cross-page object fetch
        if (meta.s.obj_size == 0) {
            //(meta.s.obj_addr & PAGE_MASK) + AtlasPtrMeta::get_size(meta) >
            //    PAGE_SIZE) {
            return true;
        }

        if (meta.s.obj_addr <=
            BKS_PSF_VA_BASE - BKS_PSF_MAX_MEM_MB * 1024 * 1024) {
            meta_.metadata_.s.obj_size = 0;
            return true;
        }

        unsigned psf_index =
            (BKS_PSF_VA_BASE - meta.s.obj_addr) >> BKS_PSF_CHUNK_SHIFT;

        /* paging-in */
        if (!global_psf[psf_index].psf) {
            if (kRefcnt) {
                meta_.inc_refcnt<card_prof>();
            }
            return true;
        }
        return false;
    }

    FORCE_INLINE void *deref_get() {
        AtlasPtrMeta::meta_t meta = {
            .u64 = meta_.load(),
        };

        if (should_paging<true, true>(meta))
            return (void *)meta_.get_object_addr();

        /* object-in */
        return deref_get_slow_path<true>(meta.s.obj_addr);
    }

    FORCE_INLINE void *deref_get_no_card() {
        AtlasPtrMeta::meta_t meta = {
            .u64 = meta_.load(),
        };

        if (should_paging<true, false>(meta))
            return (void *)meta_.get_object_addr();

        /* object-in */
        return deref_get_slow_path<false>(meta.s.obj_addr);
    }

    FORCE_INLINE void *deref_no_scope() {
        AtlasPtrMeta::meta_t meta = {
            .u64 = meta_.load(),
        };

        if (should_paging<false, true>(meta))
            return (void *)meta_.get_object_addr();

        /* object-in */
        return deref_no_scope_slow_path(meta.s.obj_addr);
    }

    FORCE_INLINE void *deref_get_paging() {
        BARRIER_ASSERT(false);
        return nullptr;
    }

    FORCE_INLINE void deref_put(void *ptr) {
        AtlasPtrMeta::meta_t meta = {
            .u64 = meta_.load(),
        };

        // Shi: Allow cross-page object fetch
        if (meta.s.obj_size == 0) {
            //(meta.s.obj_addr & PAGE_MASK) + AtlasPtrMeta::get_size(meta) >
            //    PAGE_SIZE) {
            return;
        }

        deref_put_slow_path(ptr);
    };

    FORCE_INLINE void *deref_raw(int offset) {
        AtlasPtrMeta::meta_t meta = {
            .u64 = meta_.load(),
        };
        BARRIER_ASSERT(offset == 0);
        if (should_paging<false, true>(meta))
            return AtlasPtrMeta::to_ptr(meta);

        /* object-in */
        return deref_raw_slow_path(meta.s.obj_addr, offset);
    }

    FORCE_INLINE void *deref_readonly(void *dst) {
        AtlasPtrMeta::meta_t meta = {
            .u64 = meta_.load(),
        };

        if (!dst || should_paging<false, true>(meta))
            return AtlasPtrMeta::to_ptr(meta);

        return deref_readonly_slow_path(meta.s.obj_addr, dst);
    }

    FORCE_INLINE void mark_evacuation() { meta_.mark_evacuation(); }

    FORCE_INLINE void clear_evacuation() { meta_.clear_evacuation(); }

  public:
    FORCE_INLINE void nullify() { meta_.nullify(); };
};

extern void atlas_free_object(void *ptr);

template <typename T> class AtlasUniquePtr : public AtlasGenericPtr {
  public:
    AtlasUniquePtr() { nullify(); }
    ~AtlasUniquePtr() {
        // /* TODO: only free objects which are in to-space for now, how to
        // handle
        //  * * others? */
        // if (meta_.is_tospace()) {
        //     atlas_free_object((void *)meta_.get_object_addr());
        // }
    }
    AtlasUniquePtr(uint64_t object_addr, unsigned object_size)
        : AtlasGenericPtr(object_addr, object_size) {}
    AtlasUniquePtr(AtlasUniquePtr &&other) {
        meta_ = other.meta_;
        other.nullify();
    }
    FORCE_INLINE AtlasUniquePtr &operator=(AtlasUniquePtr &&other) {
        meta_.metadata_.u64 = other.meta_.metadata_.u64;
        other.nullify();
        return *this;
    }
    NOT_COPYABLE(AtlasUniquePtr);
    void reset(T *ptr, unsigned size) {
        if (meta_.is_tospace()) {
            atlas_free_object((void *)meta_.get_object_addr());
        }
        meta_.init((uint64_t)ptr, size);
    }
    void reset_t(T *ptr) { reset(ptr, sizeof(T)); }
    FORCE_INLINE T *deref_get() { return (T *)AtlasGenericPtr::deref_get(); }
    FORCE_INLINE T *deref_get_no_card() { return (T *)AtlasGenericPtr::deref_get_no_card(); }
    FORCE_INLINE T *deref_get_paging() {
        return (T *)AtlasGenericPtr::deref_get_paging();
    }
    FORCE_INLINE T *deref_no_scope() {
        return (T *)AtlasGenericPtr::deref_no_scope();
    }
    FORCE_INLINE void deref_put(T *ptr) { AtlasGenericPtr::deref_put(ptr); }
    FORCE_INLINE T *deref_raw(int offset) {
        return (T *)AtlasGenericPtr::deref_raw(offset);
    }
    FORCE_INLINE T *deref_readonly(T *dst) {
        return (T *)AtlasGenericPtr::deref_readonly(dst);
    }
    FORCE_INLINE unsigned get_size() { return meta_.get_object_size(); }
    FORCE_INLINE bool is_tospace() { return meta_.is_tospace(); }
    /* It must be very careful to copy/move atlas's smart pointer. We provides
     * `mark_evacuation` and `clear_evacuation` apis to help do this work. */
    FORCE_INLINE void mark_evacuation() { return meta_.mark_evacuation(); }
    FORCE_INLINE void clear_evacuation() { return meta_.clear_evacuation(); }
    FORCE_INLINE void update_metadata(uint64_t object_addr,
                                      unsigned object_size) {
        meta_.update_metadata(object_addr, object_size);
    }
};

/*
 * Some applications use object ptr which points to a object with internal
 * header, and the `ptr` is the address of object data, not the header (i.e. the
 * start address of the entire object). For our implementation, we need to know
 * the header size and fetch the entire object (including the header). Here we
 * provide a wrapper to do this.
 *
 */
template <typename T, typename H>
class AtlasUniquePtrWithHeader : public AtlasUniquePtr<T> {
  private:
    enum {
        kHeaderSize = sizeof(H),
    };

  public:
    AtlasUniquePtrWithHeader() : AtlasUniquePtr<T>() {}
    AtlasUniquePtrWithHeader(T *object_addr, unsigned object_size)
        : AtlasUniquePtr<T>((uintptr_t)object_addr - kHeaderSize,
                            object_size + kHeaderSize) {}

    void reset(T *ptr, unsigned size) {
        AtlasUniquePtr<T>::reset((void *)((uintptr_t)ptr - kHeaderSize),
                                 size + kHeaderSize);
    }

    FORCE_INLINE T *deref_get() {
        return (T *)((uintptr_t)AtlasUniquePtr<T>::deref_get() + kHeaderSize);
    }

    FORCE_INLINE T *deref_raw(int offset) {
        return (T *)((uintptr_t)AtlasUniquePtr<T>::deref_raw(offset) +
                     kHeaderSize);
    }
};

static_assert(
    sizeof(AtlasUniquePtrWithHeader<void, char>) ==
        sizeof(AtlasUniquePtr<void>),
    "AtlasUniquePtrWithHeader should be the same size as AtlasUniquePtr");
} // namespace atlas

void atlas_fetch_to(char *dst, char *src, size_t size);