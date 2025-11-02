
#ifndef sph_futures_h_included
#define sph_futures_h_included

/* fine-grain parallelism based on sph/thread-pool.c.
provides task objects with functions executed in threads that can be waited for to get a result value.
manages the memory of thread-pool task objects */
#include <inttypes.h>
#include <time.h>
#include <stdatomic.h>
#include <sph-sp/sph/thread-pool.h>

#ifndef sph_future_default_poll_interval
#define sph_future_default_poll_interval { 0, 200000000 }
#endif
typedef void* (*sph_future_f_t)(void*);
typedef struct {
  sph_thread_pool_task_t task;
  _Atomic uint8_t finished;
  sph_future_f_t f;
} sph_future_t;
int sph_future_init(sph_thread_pool_size_t thread_count);
void sph_future_eval(sph_thread_pool_task_t* task);
void sph_future_new(sph_future_f_t f, void* data, sph_future_t* out);
void sph_future_deinit(void);
void* sph_future_touch(sph_future_t* a);
#endif
