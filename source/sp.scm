(library (sph sp)
  (export
    sp-fft
    sp-fft-inverse
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
    sp-noise
    sp-port-channel-count
    sp-port-close
    sp-port-input?
    sp-port-position
    sp-port-position?
    sp-port-read
    sp-port-samples-per-second
    sp-port-type
    sp-port-type->reader
    sp-port-type->writer
    sp-port-type-alsa
    sp-port-type-file
    sp-port-write
    sp-port?
    sp-ports-read
    sp-ports-write
    sp-sine)
  (import
    (guile)
    (sph base)
    (sph uniform-vector)
    (except (srfi srfi-1) map))

  (load-extension "libguile-sp" "init_sp")
  (define default-random-state (random-state-from-platform))
  (define pi 3.1415926535)

  (define (sp-port-type->writer a)
    (cond ((= sp-port-type-alsa a) sp-io-alsa-write) ((= sp-port-type-file a) sp-io-file-write)))

  (define (sp-port-type->reader a)
    (cond ((= sp-port-type-alsa a) sp-io-alsa-read) ((= sp-port-type-file a) sp-io-file-read)))

  (define (sp-port-write port sample-count segments)
    "sp-port (f32vector ...) integer -> f32vector
    write segments to the channels of port. left to right"
    ((sp-port-type->writer (sp-port-type port)) port segments sample-count))

  (define (sp-port-read port sample-count) "sp-port integer -> f32vector"
    ((sp-port-type->reader (sp-port-type port)) sample-count))

  (define (sp-ports-write ports sample-count segments)
    "(sp-sport ...) (f32vector ...) integer ->
    write segments to channels of the given ports. ports and their channels left to right"
    (fold
      (l (e segments)
        (call-with-values (l () (split-at segments (sp-port-channel-count e)))
          (l (left right) (sp-port-write e left sample-count) right)))
      segments ports))

  (define (sp-ports-read ports sample-count)
    "(sp-port ...) integer -> (f32vector ...)
    read from multiple ports"
    (apply append (map (l (e) (sp-port-read e sample-count)) ports)))

  (define* (sp-sine length sample-duration radians-per-second #:optional (phase-offset 0))
    "integer integer float float float -> f32vector
    creates a segment of given length with sample values for a sine wave with the specified properties.
    this uses guiles \"sin\" procedure and not a table-lookup oscillator or a similarly reduced computation"
    (f32vector-create length
      (l (index) (sin (* radians-per-second (+ phase-offset (* index sample-duration)))))))

  (define* (sp-noise length #:optional (random random:uniform) (random-state default-random-state))
    "integer [{random-state -> real} random-state] -> f32vector
    creates a vector of random sample values, which corresponds to random frequencies.
    default is a uniform distribution that does not repeat at every run of the program.
    guile includes several random number generators, for example: random:normal, random:uniform (the default), random:exp.
    if the state is the same, the number series will be the same"
    (f32vector-create length (l (index) (random random-state)))))