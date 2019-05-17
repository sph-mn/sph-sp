(sc-comment
  "thread-pool that uses pthread condition variables to pause unused threads.
   based on the design of thread-pool.scm from sph-lib which has been stress tested in servers and digital signal processing.
   queue.c must be included beforehand")

(pre-include "inttypes.h" "pthread.h")
(pre-define-if-not-defined thread-pool-size-t uint8-t thread-pool-thread-limit 128)

(declare
  thread-pool-t
  (type
    (struct
      (queue queue-t)
      (queue-mutex pthread-mutex-t)
      (queue-not-empty pthread-cond-t)
      (size thread-pool-size-t)
      (threads (array pthread-t thread-pool-thread-limit))))
  thread-pool-task-t struct
  thread-pool-task-t
  (type
    (struct
      thread-pool-task-t
      (q queue-node-t)
      (f (function-pointer void (struct thread-pool-task-t*)))
      (data void*)))
  thread-pool-task-f-t (type (function-pointer void (struct thread-pool-task-t*))))

(define (thread-pool-destroy a) (void thread-pool-t*)
  (pthread-cond-destroy &a:queue-not-empty)
  (pthread-mutex-destroy &a:queue-mutex))

(define (thread-finish task) (void thread-pool-task-t*)
  "if enqueued call pthread-exit to end the thread it was dequeued in"
  (free task)
  (pthread-exit 0))

(define (thread-pool-enqueue a task) (void thread-pool-t* thread-pool-task-t*)
  "add a task to be processed by the next free thread.
  mutexes are used so that the queue is only ever accessed by a single thread"
  (pthread-mutex-lock &a:queue-mutex)
  (queue-enq &a:queue &task:q)
  (pthread-cond-signal &a:queue-not-empty)
  (pthread-mutex-unlock &a:queue-mutex))

(define (thread-pool-finish a no-wait discard-queue) (int thread-pool-t* uint8-t uint8-t)
  "let threads complete all currently enqueued tasks, close the threads and free resources unless no_wait is true.
  if no_wait is true then the call is non-blocking and threads might still be running until they finish the queue after this call.
  thread_pool_finish can be called again without no_wait. with only no_wait thread_pool_destroy will not be called
  and it is unclear when it can be used to free some final resources.
  if discard_queue is true then the current queue is emptied, but note
  that if enqueued tasks free their task object these tasks wont get called anymore"
  (declare exit-value void* i thread-pool-size-t size thread-pool-size-t task thread-pool-task-t*)
  (if discard-queue
    (begin
      (pthread-mutex-lock &a:queue-mutex)
      (queue-init &a:queue)
      (pthread-mutex-unlock &a:queue-mutex)))
  (set size a:size)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set task (malloc (sizeof thread-pool-task-t)))
    (if (not task) (return 1))
    (set task:f thread-finish)
    (thread-pool-enqueue a task))
  (if (not no-wait)
    (for ((set i 0) (< i size) (set i (+ 1 i)))
      (if (= 0 (pthread-join (array-get a:threads i) &exit-value))
        (begin (set a:size (- a:size 1)) (if (= 0 a:size) (thread-pool-destroy a))))))
  (return 0))

(define (thread-pool-worker a) (void* thread-pool-t*)
  "internal worker routine"
  (declare task thread-pool-task-t*)
  (label get-task
    (pthread-mutex-lock &a:queue-mutex)
    (label wait
      (sc-comment "considers so-called spurious wakeups")
      (if a:queue.size (set task (queue-get (queue-deq &a:queue) thread-pool-task-t q))
        (begin (pthread-cond-wait &a:queue-not-empty &a:queue-mutex) (goto wait))))
    (pthread-mutex-unlock &a:queue-mutex)
    (task:f task)
    (goto get-task)))

(define (thread-pool-new size a) (int thread-pool-size-t thread-pool-t*)
  "returns zero when successful and a non-zero pthread error code otherwise"
  (declare i thread-pool-size-t attr pthread-attr-t error int)
  (set error 0)
  (queue-init &a:queue)
  (pthread-mutex-init &a:queue-mutex 0)
  (pthread-cond-init &a:queue-not-empty 0)
  (pthread-attr-init &attr)
  (pthread-attr-setdetachstate &attr PTHREAD-CREATE-JOINABLE)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set error
      (pthread-create (+ i a:threads) &attr
        (convert-type thread-pool-worker (function-pointer void* void*)) (convert-type a void*)))
    (if error
      (begin
        (if (< 0 i)
          (begin
            (sc-comment "try to finish previously created threads")
            (set a:size i)
            (thread-pool-finish a #t 0)))
        (goto exit))))
  (set a:size size)
  (label exit (pthread-attr-destroy &attr) (return error)))