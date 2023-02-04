(sc-comment
  "small example that uses the core sound generators."
  "this example depends on gnuplot to be installed."
  "it should open a gnuplot window with a series of bursts of noise."
  "see exe/run-example or exe/run-example-sc for how to compile and run with gcc")

(sc-include-once "/usr/share/sph-sp/sc-macros")
(sp-include* 48000)

(define (plain) int
  "minimal example to demonstrate the core concept of event processing"
  (declare amod sp-sample-t*)
  (define duration sp-time-t 96000)
  (define event sp-event-t sp-event-null)
  (sp-event-memory-ensure event 2)
  (sc-comment "amp(t)")
  (srq (sp-samples-new duration &amod))
  (sp-array-set-1)
  (sp-path-curves-config-declare path-config 3)
  (array-set* path-config.x 0 (/ duration 2) duration)
  (array-set* path-config.y 0 1 0)
  (srq (sp-path-curves-samples-new path-config duration &amod))
  (sp-event-memory-add _event amod)
  (sc-comment "allocate sound_event_config and set options")
  (declare sound-event-config sp-sound-event-config-t*)
  (srq (sp-sound-event-config-new &sound-event-config))
  (sp-event-memory-add _event sound-event-config)
  (set
    sound-event-config:amod amod
    event.prepare sp-sound-event-prepare
    event.config sp-sound-event-config)
  (sp-render-plot* group)

  )

(sp-samples-set in value duration)
(sp-samples-set-samples in values duration)

(sp-samples-set-out in value duration)


(sc-comment "custom type for event configuration")
(sp-declare-struct-type s1-c-t (amp sp-sample-t))

(sp-define-event* s1
  (declare c s1-c-t se-c sp-sound-event-config-t* amod sp-sample-t*)
  (set c (pointer-get (convert-type _event:config s1-c-t*)))
  (sp-event-memory-ensure _event 2)
  (sc-comment "envelope")
  (sp-path-curves-config-declare amod-path 3)
  (array-set* amod-path.x 0 (/ _duration 20) _duration)
  (array-set* amod-path.y 0 c.amp 0)
  (srq (sp-path-curves-samples-new amod-path _duration &amod))
  (sp-event-memory-add _event amod)
  (sc-comment "sound event")
  (srq (sp-sound-event-config-new &se-c))
  (sp-event-memory-add _event se-c)
  (struct-pointer-set se-c amod amod noise 1)
  (struct-set (array-get se-c:channel-config 0) use 1 amp (* 0.5 c.amp))
  (sp-sound-event _event se-c))

(sp-define-group* (t1 (rts 1 1))
  (sc-comment
    "defines a group named t1 with a default duration of (1/1 * sample_rate).
     srq (alias for status_require) checks return codes and jumps to a label named 'exit' on error")
  (declare
    s1-c s1-c-t*
    tempo sp-time-t
    times-length sp-time-t
    times (array sp-time-t 4 0 2 4 6)
    event sp-event-t)
  (set times-length 4 tempo (/ (rt 1 1) 8))
  (sp-event-memory-ensure _event times-length)
  (sp-for-each-index i times-length
    (set event s1-event)
    (sp-event-malloc-type &event s1-c-t &s1-c)
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