(sc-comment
  "small example that uses the core sound generators."
  "this example depends on gnuplot to be installed."
  "it should open a gnuplot window with a series of bursts of noise."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(pre-include "sph-sp.h")
(pre-define _sp-rate 48000)

(define (simple-event-plot) status-t
  "example for demonstrating fundamental event processing.
   plots the samples of a 10hz sine wave"
  status-declare
  (declare amod sp-sample-t*)
  (define duration sp-time-t _sp-rate event sp-event-t sp-event-null)
  (sc-comment "allocate array for loudness over time")
  (srq (sp-samples-new duration &amod))
  (sp-samples-set amod duration 1)
  (srq (sp-event-memory-add &event amod))
  (sc-comment "allocate sound_event_config, set options and finish event preparation")
  (declare sound-event-config sp-sound-event-config-t*)
  (srq (sp-sound-event-config-new &sound-event-config))
  (srq (sp-event-memory-add &event sound-event-config))
  (set
    sound-event-config:amod amod
    sound-event-config:frq 10
    sound-event-config:noise 0
    event.prepare sp-sound-event-prepare
    event.config sound-event-config
    event.start 0
    event.end duration)
  (srq (sp-render-plot event))
  (label exit status-return))

(sc-comment
  "demonstration of the use of event groups, paths, and custom types for event configuration")

(sp-declare-struct-type s1-c-t (amp sp-sample-t))

(sp-define-event* s1
  (sc-comment
    "defines a global event variable (s1-event, sound-1) using an additionally defined event-prepare function with the following content.")
  (sc-comment "custom configuration passed via the event object")
  (define c s1-c-t (pointer-get (convert-type _event:config s1-c-t*)))
  (sc-comment "envelope from an interpolated path")
  (declare amod sp-sample-t*)
  (sp-path-curves-config-declare amod-path 3)
  (array-set* amod-path.x 0 (/ _duration 10) _duration)
  (array-set* amod-path.y 0 c.amp 0)
  (srq (sp-path-curves-samples-new amod-path _duration &amod))
  (srq (sp-event-memory-add _event amod))
  (sc-comment "sound event configuration")
  (declare se-c sp-sound-event-config-t*)
  (srq (sp-sound-event-config-new &se-c))
  (srq (sp-event-memory-add _event se-c))
  (struct-pointer-set se-c amod amod frq 300)
  (struct-set (array-get se-c:channel-config 0) use 1 amp (* 0.5 c.amp))
  (sp-sound-event _event se-c))

(sp-define-group* (t1 (rts 3 1))
  (sc-comment
    "defines a group named t1 (track 1) with a default duration of 3/1 * _sp_rate.
     srq (alias for status_require) checks return error codes and jumps to a label named 'exit' on error")
  (declare
    event sp-event-t
    s1-c s1-c-t*
    tempo sp-time-t
    times-length sp-time-t
    times (array sp-time-t 8 0 2 4 6 8 12 14 16))
  (sp-event-reset event)
  (set times-length 8 tempo (/ (rt 1 1) 8))
  (sp-for-each-index i times-length
    (set event s1-event)
    (sp-event-malloc-type &event s1-c-t &s1-c)
    (set s1-c:amp (if* (modulo i 2) 0.25 1.0))
    (struct-set event
      config s1-c
      start (+ _sp-rate (* tempo (array-get times i)))
      end (+ event.start (rt 1 6)))
    (srq (sp-group-add _event event))))

(define (main) int
  status-declare
  (sc-comment "use one cpu core and two output channels")
  (sp-initialize 1 2 _sp-rate)
  (srq (sp-render-file t1-event "/tmp/sp-example.wav"))
  (label exit status-i-return))