(sc-comment
  "small example that uses the core sound generators."
  "this example depends on gnuplot to be installed."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(pre-include "sph-sp.h")
(pre-define _sp-rate 48000)

(define (simple-event-plot) status-t
  "example demonstrating fundamental event processing.
   plots the samples of a 10hz sine wave"
  status-declare
  (declare amod sp-sample-t*)
  (define duration sp-time-t _sp-rate event sp-event-t sp-null-event)
  (sc-comment "allocate array for loudness over time")
  (srq (sp-samples-new duration &amod))
  (sp-samples-set amod duration 1)
  (srq (sp-event-memory-add &event amod))
  (sc-comment "allocate sound_event_config, set options and finish event preparation")
  (declare wave-event-config sp-wave-event-config-t*)
  (srq (sp-wave-event-config-new &wave-event-config))
  (srq (sp-event-memory-add &event wave-event-config))
  (set
    wave-event-config:channel-config:amod amod
    wave-event-config:channel-config:frq 10
    event.start 0
    event.end duration)
  (sp-wave-event &event wave-event-config)
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
  (sp-event-envelope-zero3-srq _event &amod _duration 0.1 c.amp)
  (sc-comment "sound event configuration")
  (declare wec sp-wave-event-config-t*)
  (srq (sp-wave-event-config-new &wec))
  (srq (sp-event-memory-add _event wec))
  (struct-pointer-set wec:channel-config amod amod frq 300)
  (struct-pointer-set (+ 1 wec:channel-config) use 1 amp (* 0.5 c.amp))
  (sp-wave-event _event wec))

(sp-define-group* (t1 (sp-duration 3 1))
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
  (set times-length 8 tempo (/ (sp-duration 1 1) 8))
  (sp-for-each-index i times-length
    (set event s1-event)
    (sp-event-malloc-type-srq &event s1-c-t &s1-c)
    (set s1-c:amp (if* (modulo i 2) 0.25 0.9))
    (struct-set event
      config s1-c
      start (+ _sp-rate (* tempo (array-get times i)))
      end (+ event.start (sp-duration 1 6)))
    (srq (sp-group-add _event event))))

(define (main) int
  status-declare
  (sc-comment "use one cpu core and two output channels")
  (sp-initialize 1 2 _sp-rate)
  (sc-comment (srq (simple-event-plot)))
  (srq (sp-render-file t1-event "/tmp/sp-example.wav"))
  (label exit (sp-deinitialize) status-i-return))