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
            (begin (sp-io-file-set-position file-out 0)
              (sp-io-file-write file-out data test-env-segment-size)))
          (let (data-read (sp-io-file-read file-in test-env-segment-size))
            (assert-true (equal? data data-read)))
          (require-success "close" (and (sp-io-port-close file-in) (sp-io-port-close file-out))))))))

(execute-tests (ql file))
(if (file-exists? test-env-path) (delete-file test-env-path))
(define samples-per-second 8000)
(define (samples->seconds samples samples-per-second) (/ samples samples-per-second))
