(import (sph) (sph test) (sph sp) (sph list) (rnrs bytevectors))

(define* (require-success title input-result #:optional c)
  (if (error? input-result) (test-fail title input-result) (if c (c input-result) input-result)))

(define test-env-file-path "/tmp/sp-test.au")
(define test-env-segment-size 8)
(define test-env-channel-count 3)

(define (test-file) (if (file-exists? test-env-file-path) (delete-file test-env-file-path))
  (let*
    ( (file-out (sp-io-file-open-output test-env-file-path test-env-channel-count 8000))
      (file-in (sp-io-file-open-input test-env-file-path test-env-channel-count 8000))
      (data
        (n-times-map test-env-channel-count (l (n) (make-f32vector test-env-segment-size (+ n 1))))))
    (require-success "open" file-out
      (l (file-out)
        (assert-and
          (assert-equal (list #t #f 0 #t 2)
            (list (sp-port? file-out) (sp-port-input? file-out)
              (sp-port-position file-out) (sp-io-file-set-position file-out 2)
              (sp-port-position file-out)))
          (require-success "file-write"
            (begin (sp-io-file-set-position file-out 0)
              (sp-io-file-write file-out data test-env-segment-size)))
          (let (data-read (sp-io-file-read file-in test-env-segment-size))
            (assert-true (equal? data data-read)))
          (require-success "close" (and (sp-port-close file-in) (sp-port-close file-out))))))))

;(execute-tests (ql file))
(if (file-exists? test-env-file-path) (delete-file test-env-file-path))
(define (samples->seconds samples samples-per-second) (/ samples samples-per-second))

(define (create-signal-duration->file duration-seconds path channels samples-per-second proc)
  (let*
    ( (segment-size samples-per-second)
      (port (sp-io-file-open-output path channels samples-per-second)))
    (sp-io-stream #f port
      segment-size samples-per-second
      (l (offset) (if (< offset (* duration-seconds samples-per-second)) (proc offset) #f)))))

(define samples-per-second 8000)
(define segment-size samples-per-second)

(create-signal-duration->file 2 test-env-file-path
  1 segment-size
  (l (offset)
    (list
      (sp-product
        (sp-sum~ (sp-sine segment-size offset 1300 (/ 1 samples-per-second) 0)
          (sp-sine segment-size offset 2600 (/ 1 samples-per-second) 0))
        0.1))))