(library (sph sp)
  (export
    sp-fft
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
    sp-map+one~
    sp-noise
    sp-port-channel-count
    sp-port-close
    sp-port-input?
    sp-port-position
    sp-port-position?
    sp-port-samples-per-second
    sp-port-type
    sp-port-type->reader
    sp-port-type->writer
    sp-port-type-alsa
    sp-port-type-file
    sp-port?
    sp-ports-read
    sp-ports-write
    sp-sine
    (rename (f32vector-map+one sp-map+one) (f32vector-map sp-map)))
  (import
    (guile)
    (sph base)
    (sph uniform-vector)
    (sph uniform-vector)
    (except (srfi srfi-1) map))

  (load-extension "libguile-sp" "init_sp")
  (define default-random-state (random-state-from-platform))
  (define pi 3.1415926535)

  (define (sp-port-type->writer a)
    (cond ((= sp-port-type-alsa a) sp-io-alsa-write) ((= sp-port-type-file a) sp-io-file-write)))

  (define (sp-port-type->reader a)
    (cond ((= sp-port-type-alsa a) sp-io-alsa-read) ((= sp-port-type-file a) sp-io-file-read)))

  (define (sp-port-write port segments sample-count)
    "sp-port (f32vector ...) integer -> f32vector
    write segments to the channels of port, left to right"
    ((sp-port-type->writer (sp-port-type port)) port segments sample-count))

  (define (sp-port-read port sample-count) "sp-port integer -> f32vector"
    ((sp-port-type->reader (sp-port-type port)) sample-count))

  (define (sp-ports-write ports segments sample-count)
    "(sp-sport ...) (f32vector ...) integer ->
    write segments to channels of the given ports, ports and their channels left to right"
    (fold
      (l (e segments)
        (call-with-values (l () (split-at segments (sp-port-channel-count e)))
          (l (left right) (sp-port-write e left sample-count) right)))
      segments ports))

  (define (sp-ports-read ports sample-count)
    "(sp-port ...) integer -> (f32vector ...)
    read from multiple ports"
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
                (make-error (q sp-io-stream) (q result-format) ""))))))))

  (define (sp-delay sample-offset samples delay-state)
    "f32vector list:delay-state -> list:delay-state" delay-state)

  ;ramp, segment-delay, sample-delay

  (define*
    (sp-sine length sample-offset radians-per-second seconds-per-sample #:optional (phase-offset 0))
    "integer integer float float float -> f32vector
    creates a segment of given length with sample values for a sine wave with the specified properties.
    this uses guiles \"sin\" procedure and not a table-lookup oscillator or similarly reduced computations"
    (let (r (make-f32vector length))
      (f32vector-each-index
        (l (index length)
          (f32vector-set! r index
            (sin
              (* radians-per-second (+ (* (+ sample-offset index) seconds-per-sample) phase-offset)))))
        r)
      r))

  (define* (sp-noise length #:optional (random random:uniform) (random-state default-random-state))
    "integer [{random-state -> real} random-state] -> f32vector
    creates a vector of random sample values, which corresponds to random frequencies.
    default is a uniform distribution that does not repeat at every run of the program.
    guile includes several random number generators, for example: random:normal, random:uniform (the default), random:exp.
    if the state is the same, the number series will be the same"
    (let (r (make-f32vector length))
      (let loop ((index 0))
        (if (< index length)
          (begin (f32vector-set! r index (random random-state)) (loop (+ 1 index))) r)))))