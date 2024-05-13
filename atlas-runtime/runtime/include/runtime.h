#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void runtime_init();
void runtime_exit();
void *runtime_fetch(const void *object, int object_size);
bool runtime_read(void *dst, const void *object, int object_size);

/* Do the RDMA read and return the address of the DMA page, eliminating the
 * memcpy entirely
 */
void *runtime_raw_read(const void *object, int object_size, int offset);

#ifdef __cplusplus
}
#endif