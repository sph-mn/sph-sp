(pre-include "inttypes.h")

(pre-define pointer uintptr-t
  b0 void
  b8 uint8_t
  b16 uint16_t
  b32 uint32_t
  b64 uint64_t b8_s int8_t b16_s int16_t b32_s int32_t b64_s int64_t f32-s float f64-s double)

(pre-if debug-log?
  ;used like printf. example: (debug-log "%d" 1)
  (pre-define (debug-log format ...)
    (fprintf stderr (pre-string-concat "%s:%d " format "\n") __func__ __LINE__ __VA_ARGS__))
  (pre-define (debug-log format ...) null))

(pre-define null (convert-type 0 b0))
(pre-define _readonly const)
(pre-define _noalias restrict)
(pre-define (increment-one a) (set a (+ 1 a)))
(pre-define (decrement-one a) (set a (- a 1)))
;local-memory is a allocated heap-memory registry in local variables with automated free so that
;different routine end-points, like at error occurences, can free all memory up to the point easily

(pre-define (local-memory-init size) (define _local-memory-addresses[size] b0*)
  (define _local-memory-index b8 0))

(pre-define (local-memory-add pointer)
  (set (deref _local-memory-addresses _local-memory-index) pointer
    _local-memory-index (+ 1 _local-memory-index)))

(pre-define local-memory-free
  (while _local-memory-index (decrement-one _local-memory-index)
    (free (deref (+ _local-memory-addresses _local-memory-index)))))

;local-error is a way to save errors with information at different places inside routines and to use the information in a single cleanup and error return goto-label
(pre-define local-error-init (define local-error-number b32-s local-error-module b8))

(pre-define (local-error module-identifier error-identifier)
  (set local-error-module module-identifier) (set local-error-number error-identifier) (goto error))

(pre-define local-error-assert-enable #t)
(pre-define sph 1)

(pre-if local-error-assert-enable
  (pre-define (local-error-assert module number expr) (if (not expr) (local-error module number)))
  (pre-define (local-error-assert module number expr) null))

(pre-define error-memory -1)
(pre-define error-input -2)

(define (error-description n) (char* b32-s)
  (return (cond* ((= error-memory n) "memory") ((= error-input n) "input") (else "unknown"))))

(pre-define (local-define-malloc variable-name type)
  (define variable-name type* (malloc (sizeof type)))
  (if (not variable-name) (local-error sph error-memory)))
