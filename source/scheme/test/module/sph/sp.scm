(define-test-module (test module sph sp)
  (import
    (sph uniform-vector)
    (sph number)
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
  (define test-samples (f32vector-create 200 (l (index) (sin (+ 1 index)))))

  (define (get-sine-segment index sample-offset)
    (let (data (sp-sine segment-size sample-duration 1400 (* sample-offset sample-duration)))
      (list data data)))

  (define-test (sp-file) (if (file-exists? test-env-file-path) (delete-file test-env-file-path))
    (let*
      ( (file-out (sp-file-open-output test-env-file-path channel-count sample-rate))
        (file-in (sp-file-open-input test-env-file-path channel-count sample-rate))
        (data
          ; make data for channels filled with samples of value n
          (map-integers channel-count (l (n) (make-f32vector segment-size (* (+ n 1) 0.1))))))
      (assert-and
        (assert-equal "properties" (list #t #f #t 0 #t 2)
          (list (sp-port? file-out) (sp-port-input? file-out)
            (sp-port-input? file-in) (sp-port-position file-out)
            (sp-file-set-position file-out 2) (sp-port-position file-out)))
        (begin
          ; reset position and write
          (sp-file-set-position file-out 0) (sp-file-write file-out segment-size data) #t)
        (assert-true "read" (equal? data (sp-file-read file-in segment-size)))
        (assert-true "close" (and (sp-port-close file-in) (sp-port-close file-out))))))

  (define-test (sp-alsa)
    (let (out (sp-alsa-open-output "default" channel-count sample-rate))
      (let loop ((index 0) (sample-offset 0))
        (if (< sample-offset (* duration sample-rate))
          (begin
            (sp-alsa-write out segment-size
              (adjust-volume volume (get-sine-segment index sample-offset)))
            (loop (+ 1 index) (+ segment-size sample-offset)))
          (sp-port-close out)))))

  (define-test (sp-fft) (f32vector? (sp-fft-inverse (sp-fft test-samples))))

  (define-test (sp-moving-average in ex)
    (apply
      (l (source prev next distance . a)
        (let*
          ( (source (list->f32vector source)) (prev (and prev (list->f32vector prev)))
            (next (and next (list->f32vector next))) (result (f32vector-copy-empty source)))
          (apply sp-moving-average! result source prev next distance a)
          (if (every (l (a b) (float-nearly-equal? a b 1.0e-10)) ex (f32vector->list result)) ex
            result)))
      in))

  (test-execute-procedures-lambda
    (sp-moving-average
      ; no prev/next
      ((2 2 2 2) #f #f 1) (1.3333333333333333 2.0 2.0 1.3333333333333333)
      ; no prev/next, bigger width
      ((1 2.5 4.0 1.5 -3.0) #f #f 2) (1.5 1.8 1.2 1.0 0.5)
      ; prev/next
      ((2 1 0 3) (8 4) (5 9) 4)
      (2.555555582046509 3.555555582046509 3.555555582046509 2.6666667461395264)
      ; no next but prev
      ((2 1 0 3) (8 4) #f 4) (2.0 2.0 2.0 1.1111111640930176)
      ; no prev but next
      ((2 1 0 3) #f (5 9) 4)
      (1.2222222089767456 2.222222328186035 2.222222328186035 2.222222328186035))
    ;sp-file
    ;sp-fft
    ))
