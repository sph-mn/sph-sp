(sc-comment
  "small example that shows how to use the core sound generators."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a series of bursts of noise."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(sp-init* 48000)
(sp-declare-struct-type s1-c-t (amp sp-sample-t))

(sp-define-event* s1
  (declare n1-c sp-noise-event-config-t*)
  (sp-event-config-take* c s1-c-t _event)
  (sp-event-memory* _event 2)
  (sp-define-path-samples* amod ((/ _duration 20) c.amp) (_duration 0))
  (sp-event-memory-new* _event sp-noise-event-config-new n1-c)
  (struct-pointer-set n1-c amod amod)
  (sp-channel-config-set* n1-c:channel-config (0 use 1 amp (* 0.5 c.amp)))
  (sp-noise-event _event n1-c))

(sc-comment
  "defines a group named t1 with a default duration of (2/1 * sample_rate).
   srq (alias for status_require) checks return codes and jumps to an exit label on error")

(sp-define-group* (t1 (rts 1 1))
  (declare
    s1-c s1-c-t*
    tempo sp-time-t
    times-length sp-time-t
    times (array sp-time-t 4 0 2 4 6)
    event sp-event-t)
  (set times-length 4 tempo (/ (rt 1 1) 8))
  (sp-event-memory* _event times-length)
  (sp-for-each-index i times-length
    (set event s1-event)
    (sp-event-memory* &event 1)
    (sp-event-malloc-type* &event s1-c-t s1-c)
    (set s1-c:amp (if* (modulo i 2) 0.5 1.0))
    (struct-set event config s1-c start (* tempo (array-get times i)) end (+ event.start (rt 1 6)))
    (sp-group-add* _event event)))

(define (main) int
  "use one cpu core and two output channels and add one track to the song at specified offset, duration and volume"
  status-declare
  (sp-declare-group group)
  (sp-declare-event event)
  (sp-initialize 1 2 _sp-rate)
  (set event t1-event)
  (sp-group-add* &group event)
  (sp-render-plot* group)
  (label exit status-i-return))