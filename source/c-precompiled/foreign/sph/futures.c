/* fine-grain parallelism based on thread-pool.c.
provides task objects with functions executed in threads that can be waited for to get a result value.
manages the memory of thread-pool task objects.
thread-pool.c must be included beforehand */
/* for nanosleep */
#include <time.h>
uint8_t sph_futures_pool_is_initialised = 0;
thread_pool_t sph_futures_pool;
typedef void* (*future_f_t)(void*);
typedef struct {
  thread_pool_task_t task;
  uint8_t finished;
  future_f_t f;
} future_t;
/** call once to initialise the future thread pool that persists for
  the whole process or until future-deinit is called.
  can be called multiple times and just returns if the thread pool already exists.
  returns zero on success */
int future_init(thread_pool_size_t thread_count) {
  int status;
  if (sph_futures_pool_is_initialised) {
    return (0);
  } else {
    status = thread_pool_new(thread_count, (&sph_futures_pool));
    if (0 == status) {
      sph_futures_pool_is_initialised = 1;
    };
    return (status);
  };
}
/** internal future worker.
  a->f returns because modifying data likely needs extra type conversions inside a->f.
  thread-pool does not have a finished field by default so that tasks can themselves free
  their object when they finish */
void future_eval(thread_pool_task_t* task) {
  future_t* a;
  a = ((future_t*)((((uint8_t*)(task)) - offsetof(future_t, task))));
  task->data = (a->f)((task->data));
  a->finished = 1;
}
/** prepare a future in out and possibly start evaluation in parallel.
  the given function receives data as its sole argument */
void future_new(future_f_t f, void* data, future_t* out) {
  out->finished = 0;
  out->f = f;
  out->task.f = future_eval;
  out->task.data = data;
  thread_pool_enqueue((&sph_futures_pool), (&(out->task)));
}
/** can be called to stop and free the main thread-pool.
  waits till all active futures are finished */
void future_deinit() {
  thread_pool_finish((&sph_futures_pool), 0, 0);
  thread_pool_destroy((&sph_futures_pool));
}
const struct timespec sleep_time = { 0, 200000000 };
/** blocks until future is finished and returns its result */
void* touch(future_t* a) {
loop:
  if (a->finished) {
    return ((a->task.data));
  } else {
    /* poll several times per second. maybe using condition variables would be more efficient */
    nanosleep((&sleep_time), 0);
    goto loop;
  };
}
