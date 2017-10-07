(library (sph sp)
  (export
    sp-alsa-open-input
    sp-alsa-open-output
    sp-alsa-read
    sp-alsa-write
    sp-fft
    sp-fft-inverse
    sp-file-open-input
    sp-file-open-output
    sp-file-read
    sp-file-set-position
    sp-file-write
    sp-moving-average!
    sp-noise
    sp-port-channel-count
    sp-port-close
    sp-port-input?
    sp-port-position
    sp-port-position?
    sp-port-read
    sp-port-read-m
    sp-port-sample-rate
    sp-port-type
    sp-port-type->reader
    sp-port-type->writer
    sp-port-type-alsa
    sp-port-type-file
    sp-port-write
    sp-port-write-m
    sp-port?
    sp-sine!
    sp-sine-lq!)
  (import
    (sph)
    (sph list)
    (sph uniform-vector)
    (only (guile)
      load-extension
      random-state-from-platform
      random:uniform)
    (only (srfi srfi-1) split-at))

  (load-extension "libguile-sph-sp" "init_sp")
  (define default-random-state (random-state-from-platform))
  (define pi 3.1415926535)

  (define (sp-port-type->writer a)
    (cond ((= sp-port-type-alsa a) sp-alsa-write) ((= sp-port-type-file a) sp-file-write)))

  (define (sp-port-type->reader a)
    (cond ((= sp-port-type-alsa a) sp-alsa-read) ((= sp-port-type-file a) sp-file-read)))

  (define (sp-port-write port sample-count segments)
    "sp-port (f32vector ...) integer -> f32vector
     write segments to the channels of port. left to right"
    ((sp-port-type->writer (sp-port-type port)) port sample-count segments))

  (define (sp-port-read port sample-count) "sp-io integer -> f32vector"
    ((sp-port-type->reader (sp-port-type port)) sample-count))

  (define (sp-port-write-m ports sample-count segments)
    "(sp-port ...) (f32vector ...) integer ->
     write segments to channels of the given ports. ports and their channels left to right"
    (fold
      (l (a segments)
        (call-with-values (nullary (split-at segments (sp-port-channel-count a)))
          (l (left right) (sp-port-write a sample-count left) right)))
      segments ports))

  (define (sp-port-read-m ports sample-count)
    "(sp-port ...) integer -> (f32vector ...)
     read from multiple ports"
    (apply append (map (l (a) (sp-port-read a sample-count)) ports)))

  (define*
    (sp-noise sample-count #:optional (random random:uniform) (random-state default-random-state))
    "integer [{random-state -> real} random-state] -> f32vector
     creates a vector of random sample values, which corresponds to random frequencies.
     default is a uniform distribution that does not repeat at every run of the program.
     guile includes several random number generators, for example: random:normal, random:uniform (the default), random:exp.
     if the state is the same, the number series will be the same"
    (f32vector-create sample-count (l (index) (random random-state)))))
