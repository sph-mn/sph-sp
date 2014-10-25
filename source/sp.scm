(library (sph sp)
  (export
    sp-difference
    sp-difference~
    sp-division
    sp-division~
    sp-io-alsa-open-input
    sp-io-alsa-open-output
    sp-io-alsa-read
    sp-io-alsa-write
    sp-io-file-open-input
    sp-io-file-open-output
    sp-io-file-read
    sp-io-file-set-position
    sp-io-file-write
    sp-io-stream
    sp-port-channel-count
    sp-port-close
    sp-port-input?
    sp-port-position
    sp-port-position?
    sp-port-samples-per-second
    sp-port-type
    sp-port-type-alsa
    sp-port-type-file
    sp-port?
    sp-product
    sp-product~
    sp-sine
    sp-sum
    sp-sum~)
  (import
    (guile)
    (sph base)
    (sph uniform-vector)
    (except (srfi srfi-1) map))

  (define pi 3.1415926535)
  (load-extension "libguile-sp" "init_sp")

  (define (sp-sine r sample-offset radians-per-second seconds-per-sample phase-offset)
    (f32vector-each-index
      (l (index length)
        (f32vector-set! r index
          (sin
            (* radians-per-second (+ (* (+ sample-offset index) seconds-per-sample) phase-offset)))))
      r)
    r)

  (define (sp-sum r number) (f32vector-map+one! + number r) r)
  (define (sp-difference r number) (f32vector-map+one! - number r) r)
  (define (sp-division r number) (f32vector-map+one! / number r) r)
  (define (sp-product r number) (f32vector-map+one! * number r) r)
  (define (sp-sum~ a . b) (apply f32vector-sum! a b) a)
  (define (sp-difference~ a . b) (apply f32vector-difference! a b) a)
  (define (sp-division~ a . b) (apply f32vector-division! a b) a)
  (define (sp-product~ a . b) (apply f32vector-product! a b) a)

  (define (sp-port-type->writer a)
    (cond ((= sp-port-type-alsa a) sp-io-alsa-write) ((= sp-port-type-file a) sp-io-file-write)))

  (define (sp-port-type->reader a)
    (cond ((= sp-port-type-alsa a) sp-io-alsa-read) ((= sp-port-type-file a) sp-io-file-read)))

  (define (sp-port-write port segments sample-count)
    ((sp-port-type->writer (sp-port-type port)) port segments sample-count))

  (define (sp-port-read port sample-count)
    ((sp-port-type->reader (sp-port-type port)) sample-count))

  (define (sp-ports-write ports segments sample-count)
    (fold
      (l (e segments)
        (call-with-values (l () (split-at segments (sp-port-channel-count e)))
          (l (left right) (sp-port-write e left sample-count) right)))
      segments ports))

  (define (sp-ports-read ports sample-count)
    (apply append (map (l (e) (sp-port-read e sample-count)) ports)))

  (define (sp-io-stream input-ports output-ports segment-size samples-per-second proc . user-state)
    "boolean/(sp-port ...) boolean/(sp-port ...) integer integer any ... ->"
    (let
      ( (input-ports (if (sp-port? input-ports) (list input-ports) input-ports))
        (output-ports (if (sp-port? output-ports) (list output-ports) output-ports)))
      (let
        (proc
          (if input-ports
            (l (sample-offset) (apply proc sample-offset (sp-ports-read input-ports segment-size)))
            (l (sample-offset) (proc sample-offset))))
        (let loop ((sample-offset 0))
          (pass-if (proc sample-offset)
            (l (output-segments)
              (if (and (list? output-segments) (every f32vector? output-segments))
                (begin (sp-ports-write output-ports output-segments segment-size)
                  (loop (+ sample-offset samples-per-second)))
                (make-error (q sp-io-stream) (q result-format) "")))))))))