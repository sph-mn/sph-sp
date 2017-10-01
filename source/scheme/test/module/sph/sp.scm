(define-test-module (test module sph sp)
  (import
    (sph uniform-vector)
    (sph sp)
    (sph list)
    (rnrs bytevectors))

  (define test-env-file-path "/tmp/sp-test.au")
  (define channel-count 2)
  (define volume 0.1)
  (define duration 5)
  (define sample-rate 8000)
  (define sample-duration (/ 1 sample-rate))
  (define segment-size (inexact->exact (/ sample-rate 10)))
  (define (adjust-volume volume data) (map (l (a) (f32vector-map* * volume a)) data))

  (define (get-sine-segment index sample-offset)
    (let (data (sp-sine segment-size sample-duration 1400 (* sample-offset sample-duration)))
      (list data data)))

  (define-test (sp-file) (if (file-exists? test-env-file-path) (delete-file test-env-file-path))
    (let*
      ( (file-out (sp-io-file-open-output test-env-file-path channel-count sample-rate))
        (file-in (sp-io-file-open-input test-env-file-path channel-count sample-rate))
        (data
          ; make data for channels filled with samples of value n
          (map-integers channel-count (l (n) (make-f32vector segment-size (* (+ n 1) 0.1))))))
      (assert-and
        (assert-equal "properties" (list #t #f #t 0 #t 2)
          (list (sp-port? file-out) (sp-port-input? file-out)
            (sp-port-input? file-in) (sp-port-position file-out)
            (sp-io-file-set-position file-out 2) (sp-port-position file-out)))
        (begin
          ; reset position and write
          (sp-io-file-set-position file-out 0) (sp-io-file-write file-out segment-size data) #t)
        (assert-true "read" (equal? data (sp-io-file-read file-in segment-size)))
        (assert-true "close" (and (sp-port-close file-in) (sp-port-close file-out))))))

  (define-test (sp-alsa)
    (let (out (sp-io-alsa-open-output "default" channel-count sample-rate))
      (let loop ((index 0) (sample-offset 0))
        (if (< sample-offset (* duration sample-rate))
          (begin
            (sp-io-alsa-write out segment-size
              (adjust-volume volume (get-sine-segment index sample-offset)))
            (loop (+ 1 index) (+ segment-size sample-offset)))
          (sp-port-close out)))))

  (test-execute-procedures-lambda sp-file))
