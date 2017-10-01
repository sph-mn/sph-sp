(define-test-module (test module sph sp)
  (import
    (sph uniform-vector)
    (sph sp)
    (sph list)
    (rnrs bytevectors))

  (define test-env-file-path "/tmp/sp-test.au")
  (define channel-count 3)
  (define volume 0.1)
  (define duration 8)
  (define samples-per-second 8000)
  (define sample-duration (/ 1 samples-per-second))
  (define segment-size (inexact->exact (/ samples-per-second 10)))
  (define (adjust-volume volume data) (map (l (a) (f32vector-map* * volume a)) data))

  (define (get-sine-sample index sample-offset)
    ( (compose (if (even? (random 255)) reverse identity) list) (make-f32vector segment-size 0)
      (sp-sine segment-size sample-duration 1400 (* sample-offset sample-duration))))

  (define-test (sp-file) (if (file-exists? test-env-file-path) (delete-file test-env-file-path))
    (let*
      ( (file-out (sp-io-file-open-output test-env-file-path channel-count samples-per-second))
        (file-in (sp-io-file-open-input test-env-file-path channel-count samples-per-second))
        (data
          ; make data for channels filled with samples of value n
          (map-integers channel-count (l (n) (make-f32vector segment-size (+ n 1))))))
      (assert-and
        (assert-equal "properties" (list #t #f 0 #t 2)
          (list (sp-port? file-out) (sp-port-input? file-out)
            (sp-port-position file-out) (sp-io-file-set-position file-out 2)
            (sp-port-position file-out)))
        (begin
          ; reset position and write
          (sp-io-file-set-position file-out 0) (sp-io-file-write file-out segment-size data) #t)
        (assert-true "read" (equal? data (sp-io-file-read file-in segment-size)))
        (assert-true "close" (and (sp-port-close file-in) (sp-port-close file-out))))))

  (define (test-sp-alsa)
    (let (output-port (sp-io-alsa-open-output "default" channels samples-per-second))
      (let loop ((index 0) (sample-offset 0))
        (if (< sample-offset (* duration samples-per-second))
          (begin
            (sp-io-alsa-write output-port segment-size
              (adjust-volume volume (get-sine-sample index sample-offset)))
            (loop (+ 1 index) (+ segment-size sample-offset)))
          (sp-port-close output-port)))))

  (test-execute-procedures-lambda sp-file))
