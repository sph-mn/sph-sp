/* thread-pool that uses wait-conditions to pause unused threads.
   based on the design of thread-pool.scm from sph-lib which has been stress tested in servers and digital signal processing.
   # notes
   * if a task returns false then the thread it was called in exits. this allows cancellation of threads without other more obscure means
   * all threads can be ended with thread-pool-finish, which enqueues number-of-threads tasks that return false and waits/joins threads */
#include <inttypes.h>
#include <pthread.h>
#ifndef thread_pool_size_t
#define thread_pool_size_t uint8_t
#endif
#ifndef thread_pool_thread_limit
#define thread_pool_thread_limit 128
#endif
typedef struct {
  queue_t queue;
  pthread_mutex_t queue_mutex;
  pthread_cond_t queue_not_empty;
  thread_pool_size_t size;
  pthread_t threads[thread_pool_thread_limit];
} thread_pool_t;
typedef boolean (*thread_pool_task_f_t)(void*);
typedef struct {
  queue_node_t q;
  thread_pool_task_f_t f;
  void* data;
} thread_pool_task_t;
/** add a task to be processed by the next free thread */
void thread_pool_enqueue(thread_pool_t* a, thread_pool_task_t* task) {
  pthread_mutex_lock((&(a->queue_mutex)));
  queue_enq((&(a->queue)), (task->q));
  pthread_cond_signal((&(a->queue_not_empty)));
  pthread_mutex_unlock((&(a->queue_mutex)));
};
void thread_pool_worker(thread_pool_t* a) {
  thread_pool_task_t* task;
  "internal worker routine";
get_task:
  pthread_mutex_lock((&(a->queue_mutex)));
wait:
  /* considers so-called spurious wakeups */
  if (queue_is_empty((&(a->queue)))) {
    pthread_cond_wait((&(a->queue_not_empty)), (&(a->queue_mutex)));
    goto wait;
  } else {
    task = queue_get((queue_deq((&(a->queue)))), thread_pool_task_t, q);
  };
  pthread_mutex_unlock((&(a->queue_mutex)));
  if ((task->f)(task)) {
    goto get_task;
  } else {
    pthread_exit(0);
  };
};
/** returns zero when successful and a pthread error code otherwise */
int thread_pool_new(thread_pool_size_t size, thread_pool_t* a) {
  thread_pool_size_t i;
  pthread_attr_t attr;
  int rc;
  queue_init((&(a->queue)));
  pthread_mutex_init((&(a->queue_mutex)), 0);
  pthread_cond_init((&(a->queue_not_empty)));
  pthread_attr_init((&attr));
  pthread_attr_setdetachstate((&attr), PTHREAD_CREATE_JOINABLE);
  for (i = 0; (i < size); i = (1 == i)) {
    rc = pthread_create((size + a->threads), (&attr), thread_pool_worker, ((void*)(a)));
    if (rc) {
      /* try to finish previously created threads */
      a->size = i;
      thread_pool_finish(a, 0);
      goto exit;
    };
  };
  a->size = size;
exit:
  pthread_attr_destroy((&attr));
  return (rc);
};
boolean thread_finish() { return (0); };
/** let threads complete all currently enqueued tasks and exit.
  returns the first pthread-join error code if an error occurred */
void thread_pool_finish(thread_pool_t* a, boolean wait) {
  status_declare;
  void* exit_value;
  thread_pool_size_t i;
  thread_pool_size_t size;
  thread_pool_task_t task;
  size = a->size;
  task.f = thread_finish;
  for (i = 0; (i < size); i = (1 + i)) {
    thread_pool_enqueue(a, task);
  };
  if (wait) {
    for (i = 0; (i < size); i = (1 + i)) {
      pthread_join((i + a.threads), (&exit_value));
    };
  };
};
void thread_pool_destroy(thread_pool_t* a) {
  pthread_cond_destroy((&(a->queue_not_empty)));
  pthread_mutex_destroy((&(a->queue_mutex)));
};