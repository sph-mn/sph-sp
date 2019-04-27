(sc-comment
  "fine-grain parallelism based on sph/thread-pool.c."
  "provides task objects that can be waited for to get a result value from the task."
  "example usage: (touch (future (+ 2 3)))")

(pre-include "unistd.h")

(declare
  sph-futures-pool thread-pool-t
  future-f-t (type thread-pool-t)
  future-t
  (type
    (struct
      (finished boolean)
      (input void*)
      (output void*)
      (f future-f-t)
      (task thread-pool-task-t))))

(define (future-init thread-count) (int thread-pool-count-t)
  (return (thread-pool-new &sph-futures-pool thread-count)))

(define (future-eval task) (boolean thread-pool-task-t*)
  (declare a future-t)
  (set
    a (convert-type task:data future-t*)
    a:output (a:f a:input)
    a:finished #t)
  (return #t))

(define (future-new f data out) (uint8-t thread-pool-task-f-t void* future-t*)
  (declare a future-t*)
  (set a (malloc (sizeof future-t)))
  (if (not a) (return 1))
  (set
    a:finished #f
    a:output #f
    a:task.f future-eval
    a:task.data a)
  (thread-pool-enqueue sph-futures-pool &a.task))

(define (touch a) (void* future-t*)
  (label loop
    (if a:finished (return a:output)
      (begin
        (usleep 500000)
        (goto loop)))))