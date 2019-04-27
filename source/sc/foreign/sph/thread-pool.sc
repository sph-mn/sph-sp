(sc-comment
  "thread-pool that uses wait-conditions to pause unused threads.
   based on the design of thread-pool.scm from sph-lib which has been stress tested in servers and digital signal processing.
   # notes
   * if a task returns false then the thread it was called in exits. this allows cancellation of threads without other more obscure means
   * all threads can be ended with thread-pool-finish, which enqueues number-of-threads tasks that return false and waits/joins threads")

(pre-include "inttypes.h" "pthread.h")

(pre-define-if-not-defined
  thread-pool-size-t uint8-t
  thread-pool-thread-limit 128)

(declare
  thread-pool-t
  (type
    (struct
      (queue queue-t)
      (queue-mutex pthread-mutex-t)
      (queue-not-empty pthread-cond-t)
      (size thread-pool-size-t)
      (threads (array pthread-t thread-pool-thread-limit))))
  thread-pool-task-f-t (type (function-pointer boolean void*))
  thread-pool-task-t
  (type
    (struct
      (q queue-node-t)
      (f thread-pool-task-f-t)
      (data void*))))

(define (thread-pool-enqueue a task) (void thread-pool-t* thread-pool-task-t*)
  "add a task to be processed by the next free thread"
  (pthread-mutex-lock &a:queue-mutex)
  (queue-enq &a:queue task:q)
  (pthread-cond-signal &a:queue-not-empty)
  (pthread-mutex-unlock &a:queue-mutex))

(define (thread-pool-worker a) (void thread-pool-t*)
  (declare task thread-pool-task-t*)
  "internal worker routine"
  (label get-task
    (pthread-mutex-lock &a:queue-mutex)
    (label wait
      (sc-comment "considers so-called spurious wakeups")
      (if (queue-is-empty &a:queue)
        (begin
          (pthread-cond-wait &a:queue-not-empty &a:queue-mutex)
          (goto wait))
        (set task (queue-get (queue-deq &a:queue) thread-pool-task-t q))))
    (pthread-mutex-unlock &a:queue-mutex)
    (if (task:f task) (goto get-task)
      (pthread-exit 0))))

(define (thread-pool-new size a) (int thread-pool-size-t thread-pool-t*)
  "returns zero when successful and a pthread error code otherwise"
  (declare
    i thread-pool-size-t
    attr pthread-attr-t
    rc int)
  (queue-init &a:queue)
  (pthread-mutex-init &a:queue-mutex 0)
  (pthread-cond-init &a:queue-not-empty)
  (pthread-attr-init &attr)
  (pthread-attr-setdetachstate &attr PTHREAD-CREATE-JOINABLE)
  (for ((set i 0) (< i size) (set i (= 1 i)))
    (set rc (pthread-create (+ size a:threads) &attr thread-pool-worker (convert-type a void*)))
    (if rc
      (begin
        (sc-comment "try to finish previously created threads")
        (set a:size i)
        (thread-pool-finish a #f)
        (goto exit))))
  (set a:size size)
  (label exit
    (pthread-attr-destroy &attr)
    (return rc)))

(define (thread-finish) boolean (return #f))

(define (thread-pool-finish a wait) (void thread-pool-t* boolean)
  "let threads complete all currently enqueued tasks and exit.
  returns the first pthread-join error code if an error occurred"
  status-declare
  (declare
    exit-value void*
    i thread-pool-size-t
    size thread-pool-size-t
    task thread-pool-task-t)
  (set
    size a:size
    task.f thread-finish)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (thread-pool-enqueue a task))
  (if wait
    (for ((set i 0) (< i size) (set i (+ 1 i)))
      (pthread-join (+ i a.threads) &exit-value))))

(define (thread-pool-destroy a) (void thread-pool-t*)
  (pthread-cond-destroy &a:queue-not-empty)
  (pthread-mutex-destroy &a:queue-mutex))