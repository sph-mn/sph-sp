(includep "inttypes.h")

(define-macro pointer uintptr-t
  b0 void
  b8 uint8_t
  b16 uint16_t
  b32 uint32_t
  b64 uint64_t b8_s int8_t b16_s int16_t b32_s int32_t b64_s int64_t f32-s float f64-s double)

(pre-if debug-log?
  (define-macro (debug-log format ...)
    (fprintf stderr (pre-string-concat "%s:%d " format "\n") __func__ __LINE__ __VA_ARGS__))
  (define-macro (debug-log format ...) null))

(define-macro null (convert-type 0 b0))
(define-macro _readonly const)
(define-macro _noalias restrict)
(define-macro (increment-one a) (set a (+ 1 a)))
(define-macro (decrement-one a) (set a (- a 1)))

(define-macro (local-memory-init size) (define _local-memory-addresses[size] b0*)
  (define _local-memory-index b8 0))

(define-macro (local-memory-add pointer)
  (set (deref _local-memory-addresses _local-memory-index) pointer
    _local-memory-index (+ 1 _local-memory-index)))

(define-macro (local-memory-free)
  (while _local-memory-index (decrement-one _local-memory-index)
    (free (deref (+ _local-memory-addresses _local-memory-index)))))

(define-macro local-error-init (define local-error-number b32-s local-error-module b8))

(define-macro (local-error module-identifier error-identifier)
  (set local-error-module module-identifier) (set local-error-number error-identifier) (goto error))

(define-macro local-error-assert-enable #t)
(define-macro sph 1)

(pre-if local-error-assert-enable
  (define-macro (local-error-assert module number expr) (if (not expr) (local-error module number)))
  (define-macro (local-error-assert module number expr) null))

(define-macro error-memory -1)
(define-macro error-input -2)

(define (error-description n) (char* b32-s)
  (return (cond* ((= error-memory n) "memory") ((= error-input n) "input") (else "unknown"))))

(define-macro (local-define-malloc variable-name type)
  (define variable-name type* (malloc (sizeof type)))
  (if (not variable-name) (local-error sph error-memory)))