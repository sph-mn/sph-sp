(pre-include "sph-sp.h" "./helper.c")

(pre-if
  (= sp-sample-type-f64 sp-sample-type)
  (pre-define
    sp-sample-nearly-equal? f64-nearly-equal?
    sp-sample-array-nearly-equal? f64-array-nearly-equal?)
  (pre-if
    (= sp-sample-type-f32 sp-sample-type)
    (pre-define
      sp-sample-nearly-equal? f32-nearly-equal?
      sp-sample-array-nearly-equal? f32-array-nearly-equal?)))

(define error-margin sp-sample-t 0.1)

(define (test-base) status-t
  status-declare
  (test-helper-assert "input 0.5" (sp-sample-nearly-equal? 0.63662 (sp-sinc 0.5) error-margin))
  (test-helper-assert "input 1" (sp-sample-nearly-equal? 1.0 (sp-sinc 0) error-margin))
  (test-helper-assert
    "window-blackman 1.1 20"
    (sp-sample-nearly-equal? 0.550175 (sp-window-blackman 1.1 20) error-margin))
  (label exit
    (return status)))

(define (test-spectral-inversion-ir) status-t
  status-declare
  (declare
    a-len size-t
    a (array sp-sample-t 5 0.1 -0.2 0.3 -0.2 0.1))
  (set a-len 5)
  (sp-spectral-inversion-ir a a-len)
  (test-helper-assert
    "result check"
    (and
      (sp-sample-nearly-equal? -0.1 (array-get a 0) error-margin)
      (sp-sample-nearly-equal? 0.2 (array-get a 1) error-margin)
      (sp-sample-nearly-equal? -0.3 (array-get a 2) error-margin)
      (sp-sample-nearly-equal? 0.2 (array-get a 3) error-margin)
      (sp-sample-nearly-equal? -0.1 (array-get a 4) error-margin)))
  (label exit
    (return status)))

(define (test-spectral-reversal-ir) status-t
  status-declare
  (declare
    a-len size-t
    a (array sp-sample-t 5 0.1 -0.2 0.3 -0.2 0.1))
  (set a-len 5)
  (sp-spectral-reversal-ir a a-len)
  (test-helper-assert
    "result check"
    (and
      (sp-sample-nearly-equal? 0.1 (array-get a 0) error-margin)
      (sp-sample-nearly-equal? 0.2 (array-get a 1) error-margin)
      (sp-sample-nearly-equal? 0.3 (array-get a 2) error-margin)
      (sp-sample-nearly-equal? 0.2 (array-get a 3) error-margin)
      (sp-sample-nearly-equal? 0.1 (array-get a 4) error-margin)))
  (label exit
    (return status)))

(define test-file-path uint8-t* "/tmp/test-sph-sp-file")

(define (test-port) status-t
  status-declare
  (declare
    position size-t
    channel-count uint32-t
    sample-rate uint32-t
    sample-count size-t
    channel-data sp-sample-t**
    unequal int8-t
    channel-data-2 sp-sample-t**
    channel size-t
    port sp-port-t
    len size-t)
  (local-memory-init 2)
  (if (file-exists? test-file-path) (unlink test-file-path))
  (set
    channel-count 2
    sample-rate 8000
    sample-count 5
    channel-data (sp-alloc-channel-array channel-count sample-count)
    channel-data-2 (sp-alloc-channel-array channel-count sample-count))
  (sp-alloc-require channel-data)
  (local-memory-add channel-data)
  (local-memory-add channel-data-2)
  (set channel channel-count)
  (while channel
    (dec channel)
    (set len sample-count)
    (while len
      (dec len)
      (set (array-get (array-get channel-data channel) len) len)))
  ; -- test create
  (status-require (sp-file-open &port test-file-path channel-count sample-rate))
  (printf " create\n")
  (set position 0)
  (status-require (sp-port-position &position (&port)))
  (status-require (sp-port-write &port sample-count channel-data))
  (status-require (sp-port-position &position (&port)))
  (test-helper-assert "sp-port-position file after write" (= sample-count position))
  (status-require (sp-port-set-position &port 0))
  (sp-port-read channel-data-2 &port sample-count)
  ; compare read result with output data
  (set len channel-count)
  (set unequal 0)
  (while (and len (not unequal))
    (dec len)
    (set unequal
      (not
        (sp-sample-array-nearly-equal?
          (array-get channel-data len)
          sample-count (array-get channel-data-2 len) sample-count error-margin))))
  (test-helper-assert "sp-port-read new file result" (not unequal))
  (status-require (sp-port-close &port))
  (printf "  write\n")
  ; -- test open
  (status-require (sp-file-open &port test-file-path 2 8000))
  (status-require (sp-port-position &position &port))
  (test-helper-assert "sp-port-position existing file" (= sample-count position))
  (status-require (sp-port-set-position &port 0))
  (sp-port-read channel-data-2 &port sample-count)
  ; compare read result with output data
  (set
    unequal 0
    len channel-count)
  (while (and len (not unequal))
    (dec len)
    (set unequal
      (not
        (sp-sample-array-nearly-equal?
          (array-get channel-data len)
          sample-count (array-get channel-data-2 len) sample-count error-margin))))
  (test-helper-assert "sp-port-read existing result" (not unequal))
  (status-require (sp-port-close &port))
  (printf "  open\n")
  (label exit
    local-memory-free
    (return status)))

(define (test-convolve) status-t
  status-declare
  (declare
    sample-count size-t
    b-len size-t
    result-len size-t
    a-len size-t
    carryover-len size-t)
  (set
    sample-count 5
    b-len 3
    result-len sample-count
    a-len sample-count
    carryover-len b-len)
  ; allocate memory
  (local-memory-init 4)
  (sp-alloc-define-samples result result-len)
  (local-memory-add result)
  (sp-alloc-define-samples a a-len)
  (local-memory-add a)
  (sp-alloc-define-samples b b-len)
  (local-memory-add b)
  (sp-alloc-define-samples carryover carryover-len)
  (local-memory-add carryover)
  ; prepare input/output data arrays
  (array-set a 0 2 1 3 2 4 3 5 4 6)
  (array-set b 0 1 1 2 2 3)
  (array-set carryover 0 0 1 0 2 0)
  (define-array expected-result sp-sample-t (5) 2 7 16 22 28)
  (define-array expected-carryover sp-sample-t (3) 27 18 0)
  ; test convolve first segment
  (sp-convolve result a a-len b b-len carryover carryover-len)
  (test-helper-assert
    "first result"
    (sp-sample-array-nearly-equal? result result-len expected-result result-len error-margin))
  (test-helper-assert
    "first result carryover"
    (sp-sample-array-nearly-equal?
      carryover carryover-len expected-carryover carryover-len error-margin))
  ; test convolve second segment
  (array-set a 0 8 1 9 2 10 3 11 4 12)
  (array-set expected-result 0 35 1 43 2 52 3 58 4 64)
  (array-set expected-carryover 0 57 1 36 2 0)
  (sp-convolve result a a-len b b-len carryover carryover-len)
  (test-helper-assert
    "second result"
    (sp-sample-array-nearly-equal? result result-len expected-result result-len error-margin))
  (test-helper-assert
    "second result carryover"
    (sp-sample-array-nearly-equal?
      carryover carryover-len expected-carryover carryover-len error-margin))
  (label exit
    local-memory-free
    (return status)))

(define (test-moving-average) status-t
  status-declare
  (declare
    source-len size-t
    result-len size-t
    prev-len size-t
    next-len size-t
    radius size-t)
  (set source-len 8)
  (set
    result-len source-len
    prev-len 4
    next-len prev-len)
  ; allocate memory
  (local-memory-init 4)
  (sp-alloc-define-samples result result-len)
  (local-memory-add result)
  (sp-alloc-define-samples source source-len)
  (local-memory-add source)
  (sp-alloc-define-samples prev prev-len)
  (local-memory-add prev)
  (sp-alloc-define-samples next next-len)
  (local-memory-add next)
  ; set values
  (array-set source 0 1 1 4 2 8 3 12 4 3 5 32 6 2)
  (array-set prev 0 3 1 2 2 1 3 -12)
  (array-set next 0 83 1 12 2 -32 3 2)
  (set radius 4)
  ; with prev and next
  (sp-moving-average result source source-len prev prev-len next next-len 0 (- source-len 1) radius)
  ;(debug-log-samples result result-len)
  ; without prev or next
  (sp-moving-average result source source-len 0 0 0 0 0 (- source-len 1) (+ 1 (/ source-len 2)))
  (label exit
    local-memory-free
    (return status)))

(define (main) int
  status-declare
  (test-helper-test-one test-port)
  (test-helper-test-one test-convolve)
  (test-helper-test-one test-moving-average)
  (test-helper-test-one test-base)
  (test-helper-test-one test-spectral-reversal-ir)
  (test-helper-test-one test-spectral-inversion-ir)
  (label exit
    (test-helper-display-summary)
    (return status.id)))