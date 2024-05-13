#include "runtime.h"
#include "bks_ctx.h"
#include "helpers.h"
#include <bks_types.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <atomic>
#include <unistd.h>

//#define PRINT_FETCH
//#define CARD_PROFILING

extern std::atomic<unsigned long> num_fetches;
extern unsigned long last_num_fetches;
extern bool card_profiling_on;
extern struct bks_card *global_card;

paris::BksContext *bks_ctx = nullptr;

#ifdef PRINT_FETCH
void *print_fetches(void* arg) {
    while (true) {
        std::printf("num fetches: %lu\n", num_fetches.load() - last_num_fetches);
        last_num_fetches = num_fetches.load();
        sleep(1);
    }
}
#endif

#ifdef CARD_PROFILING
void *card_clock(void* arg) {
    // How many us per second is the profiling on?
    unsigned counter = 0;
    unsigned on_time = 1000000;
    unsigned off_time = 1000000 - on_time;
    while(true) {
        card_profiling_on = true;
        usleep(on_time);
        if (off_time > 0) {
            card_profiling_on = false;
            usleep(off_time);
        }
        if (counter++ % 10 == 0) {
            printf("card profiling heartbeat\n");
            fflush(stdout);
        }
    }
}
#endif

void runtime_init() {
    if (!bks_ctx)
        bks_ctx = new paris::BksContext();
    num_fetches = 0;
    last_num_fetches = 0;
#ifdef PRINT_FETCH
    pthread_t fetch_tid;
    if (pthread_create(&fetch_tid, nullptr, print_fetches, NULL) != 0) {
        printf("Error creating print fetches thread!\n");
        exit(-1);
    }
#endif
#ifdef CARD_PROFILING
    pthread_t card_tid;
    if (pthread_create(&card_tid, nullptr, card_clock, NULL) != 0) {
        printf("Error creating card clock thread!\n");
        exit(-1);
    }
    printf("This is a new version!\n");
#endif
}

void runtime_exit() {
    delete bks_ctx;
    bks_ctx = nullptr;
}

void *runtime_fetch(const void *object, int object_size) {
    BARRIER_ASSERT(bks_ctx != nullptr);
    return bks_ctx->Fetch(object, object_size);
}

bool runtime_read(void *dst, const void *object, int object_size) {
    BARRIER_ASSERT(bks_ctx != nullptr);
    return bks_ctx->Read(dst, object, object_size);
}

void *runtime_raw_read(const void *object, int object_size, int offset) {
    BARRIER_ASSERT(bks_ctx != nullptr);
    return bks_ctx->RawRead(object, object_size, offset);
}