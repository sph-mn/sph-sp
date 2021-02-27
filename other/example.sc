(sc-comment
  "small example that shows how to use the core sound generating events."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a noisy sine wave."
  "see exe/run-example for how to compile and run with gcc")

(sc-include-once "../source/sc/main/sc-macros")
(pre-include "stdio.h" "sph-sp.h")

(sp-define-trigger* example-event
  (declare
    amod sp-sample-t*
    duration sp-time-t
    s1-config sp-wave-event-config-t*
    n1-config sp-noise-event-config-t*
    n2-config sp-cheap-noise-event-config-t*)
  (sp-declare-event-3 s1 n1 n2) (free-on-error-init 4) (free-on-error _event:free _event)
  (status-require (sp-event-memory-init _event 1))
  (status-require (sp-wave-event-config-new &s1-config)) (free-on-error1 s1-config)
  (status-require (sp-noise-event-config-new &n1-config)) (free-on-error1 n1-config)
  (status-require (sp-cheap-noise-event-config-new &n2-config)) (free-on-error1 n2-config)
  (set duration (- _event:end _event:start) _event:free sp-group-free)
  (sp-path-samples* &amod duration (line (/ duration 2) 1.0) (line duration 0))
  (sp-event-memory-add1 _event &amod)
  (sp-wave* s1 0 duration *s1-config (amod amod frq 8 amp 0.5) (1 use 1 amp 0.1))
  (status-require (sp-group-add _event s1))
  (sp-cheap-noise* n2 0 duration *n2-config (amod amod cut 0.5 amp 0.1) (1 use 1 amp 0.001))
  (status-require (sp-group-add _event n2))
  (sp-noise* n1 0 duration *n1-config (amod amod cutl 0.2 cuth 0.5) (0 use 1 delay 30000 amp 1))
  (status-require (sp-group-add _event n1)) (status-require (sp-group-prepare _event)))

(define (main) int
  status-declare
  (sp-declare-event event)
  (sp-declare-event-list events)
  (sp-initialize 1 2 48000)
  (struct-set event start 0 end (* sp-rate 2) prepare example-event)
  (status-require (sp-event-list-add &events event))
  (status-require (sp-render-quick events 1))
  (label exit (return status.id)))