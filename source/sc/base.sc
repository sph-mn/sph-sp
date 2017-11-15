(pre-include-once
  stdio-h "stdio.h"
  fcntl-h "fcntl.h")

(sc-include "base.h" "base/foreign/sph/one" "base/foreign/sph/local-memory")
;
;-- other
;
(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** b32 b32)
  "return an array for channels with data arrays for each channel.
  returns zero if memory could not be allocated"
  (local-memory-init (+ channel-count 1))
  (define result sp-sample-t** (malloc (* channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0))
  (local-memory-add result)
  (define channel sp-sample-t*)
  (while channel-count
    (dec channel-count)
    (set channel (calloc (* sample-count (sizeof sp-sample-t)) 1))
    (if (not channel)
      (begin
        local-memory-free
        (return 0)))
    (local-memory-add channel)
    (set (deref result channel-count) channel))
  (return result))

(sc-include "base/io")
