(library (sph sp)
  (export
    f32vector-sum
    f64vector-sum
    float-nearly-equal?
    sp-alsa-open
    sp-duration->sample-count
    sp-file-open
    sp-pi
    sp-plot-render
    sp-port-channel-count
    sp-port-close
    sp-port-input?
    sp-port-position
    sp-port-position?
    sp-port-read
    sp-port-sample-rate
    sp-port-set-position
    sp-port-write
    sp-port?
    sp-sample-count->duration
    sp-segments->alsa
    sp-segments->file
    sp-segments->plot
    sp-segments->plot-render
    sp-sine!
    sp-sine-lq!)
  (import
    (guile)
    (sph)
    (sph process)
    (sph string))

  (load-extension "libguile-sph-sp" "sp_init_guile")
  (define sp-pi (* 4 (atan 1)))
  (define (sp-duration->sample-count seconds sample-rate) (* seconds sample-rate))
  (define (sp-sample-count->duration sample-count sample-rate) (/ sample-count sample-rate))

  (define (sp-segments->file a path sample-rate) "((vector ...) ...) string ->"
    (if (not (null? a))
      (let (out (sp-file-open path (length (first a)) sample-rate))
        (each (l (a) (sp-port-write out a)) a) (sp-port-close out))))

  (define* (sp-segments->alsa a sample-rate #:optional (device "default") (latency 4096))
    "((vector ...) ...) ->"
    (if (not (null? a))
      (let (out (sp-alsa-open device #f (length (first a)) sample-rate latency))
        (each (l (a) (sp-port-write out a)) a) (sp-port-close out))))

  (define (sp-segments->plot a path)
    "((vector ...) ...) string ->
     write gnuplot compatible sample data to file at path"
    (call-with-output-file path
      (l (file)
        (each
          (l (channel)
            (each
              (l (segment)
                (each (l (sample) (display sample file) (newline file)) (f64vector->list segment)))
              channel)
            (newline file))
          a))))

  (define (sp-plot-render file-path)
    (process-replace-p "gnuplot" "--persist"
      "-e" (string-append "set yrange [-1:1]; plot " (string-quote file-path) " with lines")))

  (define (sp-segments->plot-render a path) (sp-segments->plot a path) (sp-plot-render path)))
