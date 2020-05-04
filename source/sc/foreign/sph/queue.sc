(sc-comment
  "a fifo queue with the operations enqueue and dequeue that can enqueue custom struct types and a mix of types.
   # example usage
   typedef struct {
     // custom field definitions ...
     queue_node_t queue_node;
   } element_t;
   element_t e;
   queue_t q;
   queue_init(&q);
   queue_enq(&q, &e.queue_node);
   queue_get(queue_deq(&q), element_t, queue_node);")

(pre-include "stdlib.h" "inttypes.h" "stddef.h")

(pre-define
  queue-size-t uint32-t
  (queue-get node type field)
  (begin
    "returns a pointer to the enqueued struct based on the offset of the queue_node_t field in the struct.
     because of this queue nodes dont have to be allocated separate from user data.
     downside is that the same user data object cant be contained multiple times"
    (convert-type (- (convert-type node char*) (offsetof type field)) type*)))

(declare
  queue-node-t struct
  queue-node-t (type (struct queue-node-t (next (struct queue-node-t*))))
  queue-t (type (struct (size queue-size-t) (first queue-node-t*) (last queue-node-t*))))

(define (queue-init a) (void queue-t*)
  "initialise a queue or remove all elements"
  (set a:first 0 a:last 0 a:size 0))

(define (queue-enq a node) (void queue-t* queue-node-t*)
  "enqueue a node. the node must not already be in the queue"
  (if a:first (set a:last:next node) (set a:first node))
  (set a:last node node:next 0 a:size (+ 1 a:size)))

(define (queue-deq a) (queue-node-t* queue-t*)
  "queue must not be empty. a.size can be checked to see if the queue is empty"
  (declare n queue-node-t*)
  (set n a:first)
  (if (not n:next) (set a:last 0))
  (set a:first n:next a:size (- a:size 1))
  (return n))