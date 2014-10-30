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
    sp-port-samples-per-second
    sp-port-type
    sp-port-type->reader
    sp-port-type->writer
    sp-port-type-alsa
    sp-port-type-file
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

  (define (sp-port-write port segments sample-count)
    "sp-port (f32vector ...) integer -> f32vector
    write segments to the channels of port. left to right"
    ((sp-port-type->writer (sp-port-type port)) port segments sample-count))

  (define (sp-port-read port sample-count) "sp-port integer -> f32vector"
    ((sp-port-type->reader (sp-port-type port)) sample-count))

  (define (sp-ports-write ports segments sample-count)
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

  (define (sp-io-stream-with-segment-size+duration samples-per-second segment-size-divisor c)
    (let (segment-size (/ samples-per-second segment-size-divisor))
      (if (integer? segment-size) (c segment-size (/ samples-per-second segment-size))
        (error-create (q sp-io-stream) (q segment-size-is-not-an-integer)
          "the argument for segment-size-divisor must fullfill the assertion (integer? (/ samples-per-second segment-size-divisor))"))))

  (define (sp-io-stream-with-proc+ports proc input-ports output-ports segment-size c)
    (c
      (if input-ports
        (let (input-ports (if (sp-port? input-ports) (list input-ports) input-ports))
          (l (sample-offset) (apply proc sample-offset (sp-ports-read input-ports segment-size))))
        (l (sample-offset) (proc sample-offset)))
      (if (sp-port? output-ports) (list output-ports) output-ports)))

  (define (sp-io-stream-loop segment-size segment-duration proc output-ports user-state)
    (let loop ((time 0) (user-state user-state))
      (pass-if (apply proc time user-state)
        (l (output-segments)
          (error-require (and (list? output-segments) (every f32vector? output-segments))
            (sp-ports-write output-ports output-segments segment-size)
            (loop (+ time segment-duration)))))))

  (define
    (sp-io-stream input-ports output-ports samples-per-second segment-size-divisor proc .
      user-state)
    "#f/(sp-port ...) #f/(sp-port ...) integer number {time:seconds any ... -> #f/(f32vector:channel-data ...)} any ... ->
    maps over time to read channel-data from input-ports, if input-ports where given, and use  \"proc\" to create channel-data to be written to output-ports.
    the assertion (integer? (/ samples-per-seconds segment-size-divisor)) is required to hold true to avoid the number of seconds per sample to be an irrational number.
    1, 10, 100 or 1000 are example values for segment-size-divisor that work with common sampling rates"
    (sp-io-stream-with-segment-size+duration samples-per-second segment-size-divisor
      (l (segment-size segment-duration)
        (sp-io-stream-with-proc+ports proc input-ports
          output-ports segment-size
          (l (proc output-ports)
            (sp-io-stream-loop segment-size segment-duration proc output-ports user-state))))))

  (define (sp-delay time duration segment delay-state)
    "f32vector list:delay-state -> list:delay-state" delay-state)

  (define* (sp-sine time seconds-per-sample length radians-per-second #:optional (phase-offset 0))
    "integer integer float float float -> f32vector
    creates a segment of given length with sample values for a sine wave with the specified properties.
    this uses guiles \"sin\" procedure and not a table-lookup oscillator or a similarly reduced computation"
    (f32vector-create length
      (l (index) (sin (* radians-per-second (+ time (* index seconds-per-sample) phase-offset))))))

  (define* (sp-noise length #:optional (random random:uniform) (random-state default-random-state))
    "integer [{random-state -> real} random-state] -> f32vector
    creates a vector of random sample values, which corresponds to random frequencies.
    default is a uniform distribution that does not repeat at every run of the program.
    guile includes several random number generators, for example: random:normal, random:uniform (the default), random:exp.
    if the state is the same, the number series will be the same"
    (f32vector-create length (l (index) (random random-state)))))