
/* depends on thread-pool.c */
sph_thread_pool_t sph_futures_pool;
uint8_t sph_futures_pool_is_initialized = 0;
/** call once to initialize the future thread pool that persists for
   the whole process or until sph_future_deinit is called.
   can be called multiple times and just returns if the thread pool already exists.
   returns zero on success */
int sph_future_init(sph_thread_pool_size_t thread_count) {
  int status;
  if (sph_futures_pool_is_initialized) {
    return (0);
  } else {
    status = sph_thread_pool_new(thread_count, (&sph_futures_pool));
    if (0 == status) {
      sph_futures_pool_is_initialized = 1;
    };
    return (status);
  };
}

/** internal future worker.
   thread-pool does not have a finished field by default so that tasks can themselves free
   their object when they finish */
void sph_future_eval(sph_thread_pool_task_t* task) {
  sph_future_t* a;
  a = ((sph_future_t*)((((uint8_t*)(task)) - offsetof(sph_future_t, task))));
  task->data = (a->f)((task->data));
  a->finished = 1;
}

/** prepare a future in "out" and possibly start evaluation in parallel.
   the given function receives data as its sole argument */
void sph_future_new(sph_future_f_t f, void* data, sph_future_t* out) {
  out->finished = 0;
  out->f = f;
  out->task.f = sph_future_eval;
  out->task.data = data;
  sph_thread_pool_enqueue((&sph_futures_pool), (&(out->task)));
}

/** can be called to stop and free the main thread-pool.
   waits till all active futures are finished */
void sph_future_deinit() {
  if (sph_futures_pool_is_initialized) {
    sph_thread_pool_finish((&sph_futures_pool), 0, 0);
    sph_thread_pool_destroy((&sph_futures_pool));
  };
}

/** blocks until future is finished and returns its result */
void* sph_future_touch(sph_future_t* a) {
  const struct timespec poll_interval = sph_future_default_poll_interval;
  while (!a->finished) {
    nanosleep((&poll_interval), 0);
  };
  return ((a->task.data));
}
