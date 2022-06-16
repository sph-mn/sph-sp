(sc-comment
  "small example that shows how to use the core sound generators."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a series of bursts of noise."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(sp-init* 48000)
(declare s1-config-t (type (struct (amp sp-sample-t))))

(sp-define-event* s1
  (sp-event-config* config s1-config-t)
  (sp-event-memory* 2)
  (sp-path-samples* amod ((/ _duration 20) config.amp) (_duration 0))
  (sp-noise-config* n1c)
  (sp-noise-config-new* n1c _event (amod amod) (0 use 1 amp (* 0.5 config.amp)))
  (sp-noise* _event n1c)
  (_event:prepare _event))

(sc-comment
  "defines a group named r1 with a default duration of (1/1 * sample_rate). srq (status-require) checks return codes")

(sp-define-group* (r1 (rts 1 1))
  (sp-event* event)
  (declare
    s1-config s1-config-t*
    tempo sp-time-t
    times-length sp-time-t
    times (array sp-time-t 4 0 2 4 6))
  (set times-length 4 tempo (/ (rt 1 1) 8))
  (sp-event-memory* times-length)
  (sp-for-each-index i times-length
    (set event s1-event)
    (srq (sp-event-memory-init &event 1))
    (srq (sp-malloc-type 1 s1-config-t &s1-config))
    (sp-event-memory-add &event s1-config)
    (set s1-config:amp (if* (modulo i 2) 0.5 1.0))
    (struct-set event
      config s1-config
      start (* tempo (array-get times i))
      end (+ event.start (rt 1 6)))
    (sp-group-add* _event event)))

(sc-comment
  "use one cpu core and two output channels and add one riff to the song at specified offset, duration and volume")

(sp-define-song* 1 1 (sp-group-add* _event r1-event) (sp-render-plot*))