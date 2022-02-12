(sc-comment
  "small example that shows how to use the core sound generators."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a series of bursts of noise."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(sp-init* 48000)

(sp-define-event* d7-hh
  (sp-event-memory* 10)
  (sp-path-samples* amod ((rt 1 20) _event:volume) (_duration 0))
  (sp-noise-config* n1c)
  (sp-noise-config-new* n1c _event (amod amod))
  (sp-noise* _event n1c))

(sp-define-group* d7-hh-r1
  (sp-event-memory* 10)
  (sp-time* tempo (rt 1 3))
  (sp-intervals* times tempo 0 1 1 4 4 4 1)
  (for-each-index i times-length
    (sp-group-add* _event (array-get times i) (rt 1 6) _event:volume d7-hh)))

(sc-comment
  "use one cpu core and two output channels and add one riff to the song at specified offset, duration and volume")

(sp-define-song* 1 2 (sp-group-add* _event (rt 1 16) (rt 1 1) 0.5 d7-hh-r1) (sp-render-plot*))