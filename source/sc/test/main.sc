(pre-include "sph-sp.c")
(sc-include "test/helper")
(pre-define error-margin 1.0e-6)

(define (test-base) status-t
  status-init
  (test-helper-assert "input 0.5" (float-nearly-equal? 0.63662 (sp-sinc 0.5) error-margin))
  (test-helper-assert "input 1" (float-nearly-equal? 1.0 (sp-sinc 0) error-margin))
  (test-helper-assert "window-blackman 1.1 20"
    (float-nearly-equal? 0.550175 (sp-window-blackman 1.1 20) error-margin))
  (label exit (return status)))

(define (test-spectral-inversion-ir) status-t
  status-init (define a-len size-t 5)
  (define-array a sp-sample-t (5) 0.1 -0.2 0.3 -0.2 0.1) (sp-spectral-inversion-ir a a-len)
  (test-helper-assert "result check"
    (and (float-nearly-equal? -0.1 (array-get a 0) error-margin)
      (float-nearly-equal? 0.2 (array-get a 1) error-margin)
      (float-nearly-equal? -0.3 (array-get a 2) error-margin)
      (float-nearly-equal? 0.2 (array-get a 3) error-margin)
      (float-nearly-equal? -0.1 (array-get a 4) error-margin)))
  (label exit (return status)))

(define (test-spectral-reversal-ir) status-t
  status-init (define a-len size-t 5)
  (define-array a sp-sample-t (5) 0.1 -0.2 0.3 -0.2 0.1) (sp-spectral-reversal-ir a a-len)
  (test-helper-assert "result check"
    (and (float-nearly-equal? 0.1 (array-get a 0) error-margin)
      (float-nearly-equal? 0.2 (array-get a 1) error-margin)
      (float-nearly-equal? 0.3 (array-get a 2) error-margin)
      (float-nearly-equal? 0.2 (array-get a 3) error-margin)
      (float-nearly-equal? 0.1 (array-get a 4) error-margin)))
  (label exit (return status)))

(define test-file-path b8* "/tmp/test-sph-sp-file")

(define (test-port) status-t
  status-init (local-memory-init 1)
  (if (file-exists? test-file-path) (unlink test-file-path)) (define channel-count b32 2)
  (define sample-rate b32 8000) (define port sp-port-t)
  ; not existing file
  (status-require! (sp-file-open (address-of port) test-file-path channel-count sample-rate))
  (debug-log "type %d" (struct-get port type))
  (define channel-data sp-sample-t** (sp-alloc-channel-data channel-count 5))
  (sp-status-require-alloc channel-data) (local-memory-add channel-data)
  (define sample-count b32 5) (define len size-t)
  (define channel size-t channel-count)
  (while channel (dec channel)
    (set len sample-count) (while len (dec len) (set (deref (deref channel-data channel) len) len)))
  (status-require! (sp-port-write (address-of port) sample-count channel-data))
  (status-require! (sp-port-close (address-of port)))
  ; already existing file
  (status-require! (sp-file-open (address-of port) test-file-path 2 8000))
  (status-require! (sp-port-close (address-of port))) (label exit local-memory-free (return status)))

(define (main) int
  status-init (test-helper-test-one test-port)
  ;(test-helper-test-one test-base)
  ;(test-helper-test-one test-spectral-reversal-ir)
  ;(test-helper-test-one test-spectral-inversion-ir)
  (label exit (test-helper-display-summary) (return (struct-get status id))))
