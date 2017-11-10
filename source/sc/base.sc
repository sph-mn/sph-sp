(pre-include-once
  stdio-h "stdio.h"
  fcntl-h "fcntl.h")

(sc-include "base.h" "base/foreign/sph/one" "base/foreign/sph/local-memory")
; todo: better memory interface

(pre-define (sp-define-malloc id type size)
  (define id type (malloc size))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-set-malloc id size)
  (set id (malloc size))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-define-calloc id type size)
  (define id type (calloc size 1))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-set-calloc id size)
  (set id (calloc size 1))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-define-malloc-samples id sample-count)
  (sp-define-malloc id sp-sample-t* (* sample-count (sizeof sp-sample-t))))

(pre-define (sp-set-malloc-samples id sample-count)
  (sp-set-malloc id (* sample-count (sizeof sp-sample-t))))

(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** b32 b32)
  "zero if memory could not be allocated"
  (local-memory-init (+ channel-count 1))
  (define result sp-sample-t** (malloc (* channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0))
  (local-memory-add result)
  (define channel sp-sample-t*)
  (while channel-count
    (dec channel-count)
    (set channel (calloc (* sample-count (sizeof sp-sample-t)) 1))
    (local-memory-add channel)
    (if (not channel)
      (begin
        local-memory-free
        (return 0)))
    (set (deref result channel-count) channel))
  (return result))

(sc-include "base/io")
