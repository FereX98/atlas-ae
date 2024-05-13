#pragma once
#include <immintrin.h>
#include <stdint.h>
/* Use TSX inst to check if an object is in remote.
 * Here `-O1` is necessary to make sure the memory access happens */
#pragma GCC push_options
#pragma GCC optimize("O1")
static bool __attribute__((noinline)) tsx_remote_check(const void *object) {
    bool is_local = false;
    uint8_t word = 0;
    uint32_t status = 0;
retry:
    status = _xbegin();
    if (status == _XBEGIN_STARTED) {
        word = *(uint8_t *)object;
        _xend();
        is_local = true;
    } else if (status & _XABORT_RETRY) {
        goto retry;
    } else if (status & _XABORT_CONFLICT) {
        is_local = true;
    }
    return !is_local && !word;
}
#pragma GCC pop_options