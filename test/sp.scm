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
(define sample-duration (/ 1 samples-per-second))
(define segment-size (/ samples-per-second 10))
(define channels 2)
(define output-path test-env-file-path)
(define volume 0.1)
(define (set-volume data) (map (l (e) (f32vector-map+one * volume e)) data))
(define duration 8)

(define (create index sample-offset)
  ( (compose (if (even? (random 255)) reverse identity) list) (make-f32vector segment-size 0)
    (sp-sine segment-size sample-duration 1400 (* sample-offset sample-duration))))

(let (output-port (error-exit (sp-io-alsa-open-output "default" channels samples-per-second)))
  (let loop ((index 0) (sample-offset 0))
    (if (< sample-offset (* duration samples-per-second))
      (begin
        (error-exit
          (sp-io-alsa-write output-port segment-size (set-volume (create index sample-offset))))
        (loop (+ 1 index) (+ segment-size sample-offset)))
      (sp-port-close output-port))))