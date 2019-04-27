/* fine-grain parallelism based on sph/thread-pool.c.
provides task objects that can be waited for to get a result value from the task.
example usage: (touch (future (+ 2 3))) */
#include <unistd.h>
thread_pool_t sph_futures_pool;
typedef thread_pool_t future_f_t;
typedef struct {
  boolean finished;
  void* input;
  void* output;
  future_f_t f;
  thread_pool_task_t task;
} future_t;
int future_init(thread_pool_count_t thread_count) { return ((thread_pool_new((&sph_futures_pool), thread_count))); };
boolean future_eval(thread_pool_task_t* task) {
  future_t a;
  a = ((future_t*)(task->data));
  a->output = (a->f)((a->input));
  a->finished = 1;
  return (1);
};
uint8_t future_new(thread_pool_task_f_t f, void* data, future_t* out) {
  future_t* a;
  a = malloc((sizeof(future_t)));
  if (!a) {
    return (1);
  };
  a->finished = 0;
  a->output = 0;
  a->task.f = future_eval;
  a->task.data = a;
  thread_pool_enqueue(sph_futures_pool, (&(a.task)));
};
void* touch(future_t* a) {
loop:
  if (a->finished) {
    return ((a->output));
  } else {
    usleep(500000);
    goto loop;
  };
};