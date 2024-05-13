#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct atlas_unique_ptr {
    uintptr_t handle;
} atlas_unique_ptr;

typedef struct atlas_unique_ptr_wh {
    uintptr_t handle;
} atlas_unique_ptr_wh;

atlas_unique_ptr atlas_make_unique_ptr(void *object, unsigned object_size);
void *atlas_up_force_move(atlas_unique_ptr *up);
void *atlas_up_deref_get(atlas_unique_ptr *up);
void *atlas_up_deref_get_no_card(atlas_unique_ptr *up);
void *atlas_up_deref_no_scope(atlas_unique_ptr *up);
void *atlas_up_deref_get_paging(atlas_unique_ptr *up);
void *atlas_up_deref_get_evicted(atlas_unique_ptr *up, bool *evicted);
void atlas_up_deref_put(atlas_unique_ptr *up, void *ptr);
void *atlas_up_deref_raw(atlas_unique_ptr *up, int offset);
void atlas_up_reset(atlas_unique_ptr *up, void *ptr, unsigned object_size);
void atlas_up_release(atlas_unique_ptr *up);
void atlas_up_mark_evacuation(atlas_unique_ptr *up);
void atlas_up_clear_evacuation(atlas_unique_ptr *up);

atlas_unique_ptr_wh atlas_make_unique_ptr_wh(void *object, unsigned object_size,
                                             unsigned header_size);
void *atlas_upwh_deref_get(atlas_unique_ptr_wh *up);
void atlas_upwh_deref_put(atlas_unique_ptr_wh *up, void *ptr);
void *atlas_upwh_deref_raw(atlas_unique_ptr_wh *up, int offset);
void atlas_upwh_reset(atlas_unique_ptr_wh *up, void *ptr, unsigned object_size,
                      unsigned header_size);
void atlas_upwh_release(atlas_unique_ptr_wh *up);

#ifdef __cplusplus
}
#endif
