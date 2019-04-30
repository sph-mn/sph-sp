/* thread-pool that uses pthread condition variables to pause unused threads.
   based on the design of thread-pool.scm from sph-lib which has been stress tested in servers and digital signal processing.
   queue.c must be included beforehand */
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
struct thread_pool_task_t;
typedef struct thread_pool_task_t {
  queue_node_t q;
  void (*f)(struct thread_pool_task_t*);
  void* data;
} thread_pool_task_t;
typedef void (*thread_pool_task_f_t)(struct thread_pool_task_t*);
void thread_pool_destroy(thread_pool_t* a) {
  pthread_cond_destroy((&(a->queue_not_empty)));
  pthread_mutex_destroy((&(a->queue_mutex)));
};
/** if enqueued call pthread-exit to end the thread it was dequeued in */
void thread_finish(thread_pool_task_t* task) {
  free(task);
  pthread_exit(0);
};
/** add a task to be processed by the next free thread.
  mutexes are used so that the queue is only ever accessed by a single thread */
void thread_pool_enqueue(thread_pool_t* a, thread_pool_task_t* task) {
  pthread_mutex_lock((&(a->queue_mutex)));
  queue_enq((&(a->queue)), (&(task->q)));
  pthread_cond_signal((&(a->queue_not_empty)));
  pthread_mutex_unlock((&(a->queue_mutex)));
};
/** let threads complete all currently enqueued tasks, close the threads and free resources unless no_wait is true.
  if no_wait is true then the call is non-blocking and threads might still be running until they finish the queue after this call.
  thread_pool_finish can be called again without no_wait. with only no_wait thread_pool_destroy will not be called
  and it is unclear when it can be used to free some final resources.
  if discard_queue is true then the current queue is emptied, but note
  that if enqueued tasks free their task object these tasks wont get called anymore */
int thread_pool_finish(thread_pool_t* a, uint8_t no_wait, uint8_t discard_queue) {
  void* exit_value;
  thread_pool_size_t i;
  thread_pool_size_t size;
  thread_pool_task_t* task;
  if (discard_queue) {
    pthread_mutex_lock((&(a->queue_mutex)));
    queue_init((&(a->queue)));
    pthread_mutex_unlock((&(a->queue_mutex)));
  };
  size = a->size;
  for (i = 0; (i < size); i = (1 + i)) {
    task = malloc((sizeof(thread_pool_task_t)));
    if (!task) {
      return (1);
    };
    task->f = thread_finish;
    thread_pool_enqueue(a, task);
  };
  if (!no_wait) {
    for (i = 0; (i < size); i = (1 + i)) {
      if (0 == pthread_join(((a->threads)[i]), (&exit_value))) {
        a->size = (a->size - 1);
        if (0 == a->size) {
          thread_pool_destroy(a);
        };
      };
    };
  };
  return (0);
};
/** internal worker routine */
void* thread_pool_worker(thread_pool_t* a) {
  thread_pool_task_t* task;
get_task:
  pthread_mutex_lock((&(a->queue_mutex)));
wait:
  /* considers so-called spurious wakeups */
  if (a->queue.size) {
    task = queue_get((queue_deq((&(a->queue)))), thread_pool_task_t, q);
  } else {
    pthread_cond_wait((&(a->queue_not_empty)), (&(a->queue_mutex)));
    goto wait;
  };
  pthread_mutex_unlock((&(a->queue_mutex)));
  (task->f)(task);
  goto get_task;
};
/** returns zero when successful and a non-zero pthread error code otherwise */
int thread_pool_new(thread_pool_size_t size, thread_pool_t* a) {
  thread_pool_size_t i;
  pthread_attr_t attr;
  int error;
  error = 0;
  queue_init((&(a->queue)));
  pthread_mutex_init((&(a->queue_mutex)), 0);
  pthread_cond_init((&(a->queue_not_empty)), 0);
  pthread_attr_init((&attr));
  pthread_attr_setdetachstate((&attr), PTHREAD_CREATE_JOINABLE);
  for (i = 0; (i < size); i = (1 + i)) {
    error = pthread_create((i + a->threads), (&attr), ((void* (*)(void*))(thread_pool_worker)), ((void*)(a)));
    if (error) {
      if (0 < i) {
        /* try to finish previously created threads */
        a->size = i;
        thread_pool_finish(a, 1, 0);
      };
      goto exit;
    };
  };
  a->size = size;
exit:
  pthread_attr_destroy((&attr));
  return (error);
};