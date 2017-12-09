(pre-include "sph-sp.h")
(sc-include "generic/test/helper")

(pre-if
  (= sp-sample-type-f64 sp-sample-type)
  (begin
    (pre-define
      sp-sample-nearly-equal? f64-nearly-equal?
      sp-sample-array-nearly-equal? f64-array-nearly-equal?))
  (pre-if
    (= sp-sample-type-f32 sp-sample-type)
    (begin
      (pre-define
        sp-sample-nearly-equal? f32-nearly-equal?
        sp-sample-array-nearly-equal? f32-array-nearly-equal?))))

(define error-margin sp-sample-t 0.1)

(define (test-base) status-t
  status-init
  (test-helper-assert "input 0.5" (sp-sample-nearly-equal? 0.63662 (sp-sinc 0.5) error-margin))
  (test-helper-assert "input 1" (sp-sample-nearly-equal? 1.0 (sp-sinc 0) error-margin))
  (test-helper-assert
    "window-blackman 1.1 20"
    (sp-sample-nearly-equal? 0.550175 (sp-window-blackman 1.1 20) error-margin))
  (label exit
    (return status)))

(define (test-spectral-inversion-ir) status-t
  status-init
  (define a-len size-t 5)
  (define-array a sp-sample-t (5) 0.1 -0.2 0.3 -0.2 0.1)
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
  status-init
  (define a-len size-t 5)
  (define-array a sp-sample-t (5) 0.1 -0.2 0.3 -0.2 0.1)
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

(define test-file-path b8* "/tmp/test-sph-sp-file")

(define (test-port) status-t
  status-init
  (local-memory-init 2)
  (if (file-exists? test-file-path) (unlink test-file-path))
  (define channel-count b32 2)
  (define sample-rate b32 8000)
  (define port sp-port-t)
  (define sample-count size-t 5)
  (define channel-data sp-sample-t** (sp-alloc-channel-array channel-count sample-count))
  (define channel-data-2 sp-sample-t** (sp-alloc-channel-array channel-count sample-count))
  (sp-alloc-require channel-data)
  (local-memory-add channel-data)
  (local-memory-add channel-data-2)
  (define len size-t)
  (define channel size-t channel-count)
  (while channel
    (dec channel)
    (set len sample-count)
    (while len
      (dec len)
      (set (deref (deref channel-data channel) len) len)))
  ; -- test create
  (status-require! (sp-file-open (address-of port) test-file-path channel-count sample-rate))
  (printf " create\n")
  (define position size-t 0)
  (status-require! (sp-port-position (address-of position) (address-of port)))
  (status-require! (sp-port-write (address-of port) sample-count channel-data))
  (status-require! (sp-port-position (address-of position) (address-of port)))
  (test-helper-assert "sp-port-position file after write" (= sample-count position))
  (status-require! (sp-port-set-position (address-of port) 0))
  (sp-port-read channel-data-2 (address-of port) sample-count)
  ; compare read result with output data
  (set len channel-count)
  (define unequal b8-s 0)
  (while (and len (not unequal))
    (dec len)
    (set unequal
      (not
        (sp-sample-array-nearly-equal?
          (deref channel-data len) sample-count (deref channel-data-2 len) sample-count error-margin))))
  (test-helper-assert "sp-port-read new file result" (not unequal))
  (status-require! (sp-port-close (address-of port)))
  (printf "  write\n")
  ; -- test open
  (status-require! (sp-file-open (address-of port) test-file-path 2 8000))
  (status-require! (sp-port-position (address-of position) (address-of port)))
  (test-helper-assert "sp-port-position existing file" (= sample-count position))
  (status-require! (sp-port-set-position (address-of port) 0))
  (sp-port-read channel-data-2 (address-of port) sample-count)
  ; compare read result with output data
  (set
    unequal 0
    len channel-count)
  (while (and len (not unequal))
    (dec len)
    (set unequal
      (not
        (sp-sample-array-nearly-equal?
          (deref channel-data len) sample-count (deref channel-data-2 len) sample-count error-margin))))
  (test-helper-assert "sp-port-read existing result" (not unequal))
  (status-require! (sp-port-close (address-of port)))
  (printf "  open\n")
  (label exit
    local-memory-free
    (return status)))

(define (test-convolve) status-t
  status-init
  (define sample-count size-t 5)
  (define b-len size-t 3)
  (define result-len size-t sample-count)
  (define a-len size-t sample-count)
  (define carryover-len size-t b-len)
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
  (array-set-index a 0 2 1 3 2 4 3 5 4 6)
  (array-set-index b 0 1 1 2 2 3)
  (array-set-index carryover 0 0 1 0 2 0)
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
  (array-set-index a 0 8 1 9 2 10 3 11 4 12)
  (array-set-index expected-result 0 35 1 43 2 52 3 58 4 64)
  (array-set-index expected-carryover 0 57 1 36 2 0)
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
  status-init
  (define source-len size-t 8)
  (define result-len size-t source-len)
  (define prev-len size-t 4)
  (define next-len size-t prev-len)
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
  (array-set source 1 4 8 12 3 32 2)
  (array-set prev 3 2 1 -12)
  (array-set next 83 12 -32 2)
  (define radius size-t 4)
  ; with prev and next
  (sp-moving-average result source source-len prev prev-len next next-len 0 (- source-len 1) radius)
  ;(debug-log-samples result result-len)
  ; without prev or next
  (sp-moving-average result source source-len 0 0 0 0 0 (- source-len 1) (+ 1 (/ source-len 2)))
  (label exit
    local-memory-free
    (return status)))

(define (main) int
  status-init
  (test-helper-test-one test-port)
  (test-helper-test-one test-convolve)
  (test-helper-test-one test-moving-average)
  (test-helper-test-one test-base)
  (test-helper-test-one test-spectral-reversal-ir)
  (test-helper-test-one test-spectral-inversion-ir)
  (label exit
    (test-helper-display-summary)
    (return (struct-get status id))))
