(sc-comment
  "small example that shows how to use the core sound generators."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a series of bursts of noise."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(sp-init* 48000)

(sp-define-event* s1
  (sp-event-config-load config sp-default-event-config-t _event)
  (sp-event-memory* 10)
  (sp-path-samples* amod ((rt 1 20) config.amp) (_duration 0))
  (sp-noise-config* n1c)
  (sp-noise-config-new* n1c _event (amod amod) (0 use 1 amp (* 0.5 config.amp)))
  (sp-noise* _event n1c))

(sp-define-group* (r1 (rts 1 1))
  (sp-event-memory* 10)
  (sp-time* tempo (/ (rt 1 1) 8))
  (sp-sample* volume 1)
  (sp-times-values* times 0 2 4 6)
  (for-each-index i times-length
    (sc-comment
      "the default config can be used to pass values for amplitude, frequency and panning")
    (set volume (if* (modulo i 2) 0.5 1.0))
    (sp-default-event-config-new volume 0 0 (convert-type &s1.config sp-default-event-config-t**))
    (sp-event-memory-add _event s1.config)
    (sp-group-add* _event (* tempo (array-get times i)) (rt 1 6) s1)))

(sc-comment
  "use one cpu core and two output channels and add one riff to the song at specified offset, duration and volume")

(sp-define-song* 1 2 (sp-group-add* _event 0 0 r1) (sp-render-plot*))