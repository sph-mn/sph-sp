(import (sph) (sph test) (sph sp) (sph list))

(define* (require-success title input-result #:optional c)
  (if (error? input-result) (test-fail title input-result) (if c (c input-result) input-result)))

(define test-env-path "/tmp/sp-test.au")
(define test-env-segment-size 8)
(define test-env-channel-count 3)

(define (test-file) (if (file-exists? test-env-path) (delete-file test-env-path))
  (let*
    ( (file-out (sp-io-file-open-output test-env-path test-env-channel-count 8000))
      (file-in (sp-io-file-open-input test-env-path test-env-channel-count 8000))
      (data
        (n-times-map test-env-channel-count (l (n) (make-f32vector test-env-segment-size (+ n 1))))))
    (require-success "open" file-out
      (l (file-out)
        (assert-and
          (assert-equal (list #t #f 0 #t 2)
            (list (sp-io-port? file-out) (sp-io-port-input? file-out)
              (sp-io-port-position file-out) (sp-io-file-set-position file-out 2)
              (sp-io-port-position file-out)))
          (require-success "file-write"
            (begin (sp-io-file-set-position file-out 0) (sp-io-file-write file-out data)))
          (let (data-read (sp-io-file-read file-in test-env-segment-size))
            (assert-true (equal? data data-read)))
          (require-success "close" (and (sp-io-port-close file-in) (sp-io-port-close file-out))))))))

;(execute-tests (ql file))
(if (file-exists? test-env-path) (delete-file test-env-path))
(define samples-per-second 8000)
(define pi 3.1415926535)
(define (samples->seconds samples samples-per-second) (/ samples samples-per-second))

(define (osc segment time frequency)
  (let (sample-count (f32vector-length segment))
    (let loop ((index 0))
      (if (< index sample-count)
        (begin
          (f32vector-set! segment index
            (* 0.1 (sin (* frequency (samples->seconds (+ time index) 8000)))))
          (loop (+ 1 index)))
        segment))))

(define (exit-on-error a) (if (error? a) (begin (debug-log a) (exit -1)) a))

(let
  ( (out (exit-on-error (sp-io-alsa-open-output "default" 1 samples-per-second)))
    (segment-size samples-per-second))
  (let loop ((time 0))
    (if (< time 1)
      (begin
        (exit-on-error
          (sp-io-alsa-write out (list (osc (make-f32vector segment-size) time 1556)) segment-size))
        (loop (+ time 1)))))
  (exit-on-error (sp-io-port-close out)))