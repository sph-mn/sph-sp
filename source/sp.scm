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
    sp-io-port-channel-count
    sp-io-port-close
    sp-io-port-input?
    sp-io-port-position
    sp-io-port-position?
    sp-io-port-samples-per-second
    sp-io-port?
    sp-io-stream
    sp-product
    sp-product~
    sp-sine
    sp-sum
    sp-sum~)
  (import
    (guile)
    (sph)
    (sph uniform-vector))

  (define pi 3.1415926535)
  (load-extension "libguile-sp" "init_sp")

  (define (sp-sine r sample-offset radians-per-second seconds-per-sample)
    (f32vector-each-index
      (l (index length) (sin (* radians-per-second (* (+ sample-offset index) seconds-per-sample))))
      r)
    r)

  (define (sp-sum r number) (f32vector-map+one! + number r))
  (define (sp-difference r number) (f32vector-map+one! - number r))
  (define (sp-division r number) (f32vector-map+one! / number r))
  (define (sp-product r number) (f32vector-map+one! * number r))
  (define sp-sum~ f32vector-sum!)
  (define sp-difference~ f32vector-difference!)
  (define sp-division~ f32vector-division!)
  (define sp-product~ f32vector-product!)

  (define (sp-port-type->writer a)
    (cond ((= sp-port-type-alsa) sp-io-alsa-write) ((= sp-port-type-file) sp-io-file-write)))

  (define (sp-port-type->reader a)
    (cond ((= sp-port-type-alsa) sp-io-alsa-read) ((= sp-port-type-file) sp-io-file-read)))

  (define (sp-port-write port segments sample-count)
    ((sp-port-type->writer (sp-port-type port)) port segments sample-count))

  (define (sp-port-read port sample-count)
    ((sp-port-type->reader (sp-port-type port)) sample-count))

  (define (sp-io-ports-write ports segments)
    (each (l (e) ((sp-port-type->writer (sp-port-type e)) e (take (sp-io-port-channel-count e))))
      ports))

  (define (sp-io-stream input-ports output-ports samples-per-second . user-state)
    (let loop ((sample-offset 0))
      (pass-if (apply proc sample-offset)
        (l (output-segments) (sp-io-ports-write output-ports output-segments)
          (loop (+ offset samples-per-second)))))))