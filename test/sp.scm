(import (sph) (sph uniform-vector) (sph error) (sph test) (sph sp) (sph list) (rnrs bytevectors))

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
(define samples-per-second 8000)
(define sample-duration 1/8000)
(define segment-size (/ samples-per-second 100))
(define channels 2)
(define output-path test-env-file-path)
(define volume 0.1)

(define (sp-loop segment-size proc)
  (let loop ((offset 0)) (if (proc offset) (loop (+ segment-size offset)))))

(define (create offset output-port)
  (sp-port-write output-port segment-size
    (make-list 2
      (f32vector-map+one * volume
        (f32vector-map + (sp-sine segment-size sample-duration 1300 (* offset sample-duration))
          (sp-sine segment-size sample-duration 2600 (* offset sample-duration)))))))

(let (port (sp-io-file-open-output output-path channels samples-per-second))
  (sp-loop segment-size (l (offset) (and (< offset (* 2 samples-per-second)) (create offset port))))
  (sp-port-close port))