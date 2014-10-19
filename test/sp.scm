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

(define (osc segment time)
  (let loop ((index (f32vector-length segment)))
    (f32vector-set! segment index (sin (* index time))))
  (debug-log sement))

(let ((out (sp-io-file-open-output test-env-path 1 8000)) (segment-size 8000))
  (let loop ((time 0))
    (if (< time 5)
      (begin (osc (make-f32vector segment-size) time)
        ;(sp-io-file-write out (list (osc (make-f32vector segment-size) time)) segment-size)
        (loop (+ time 1))))))