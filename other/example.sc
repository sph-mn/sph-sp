(sc-comment
  "small example that shows how to use the core sound generating events."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a noisy sine wave."
  "see exe/run-example for how to compile and run with gcc")

(sc-include-once "../source/sc/main/sc-macros")
(pre-include "stdio.h" "sph-sp.h")

(sp-define-event (example-event duration)
  sp-sample-t
  (declare amod sp-sample-t*)
  (sp-declare-event-4 g s1 n1 n2)
  (sp-declare-sine-config s1-config)
  (sp-declare-noise-config n1-config)
  (sp-declare-cheap-noise-config n2-config)
  (free-on-error-init 1)
  (status-require (sp-group-new 0 10 &g))
  (free-on-error &g g.free)
  (status-require (sp-event-memory-init g 1))
  (sp-path-samples &amod duration (line (/ duration 2) 1.0) (line duration 0))
  (sp-event-memory-add g &amod)
  (sp-sine &s1 0 duration s1-config (amod amod frq 3) (1 use 1 amp 0.1))
  (sp-group-add g s1)
  (sp-noise &n1 0 duration n1-config (amod amod cutl 0.2) (0 use 1 delay 30000 amp 0.2))
  (sp-group-add g n1)
  (sp-cheap-noise &n2 0 duration n2-config (amod amod cut 0.5) (1 use 1 amp 0.001))
  (sp-group-add g n2)
  (set _result g))

(define (main) int
  status-declare
  (sp-declare-event event)
  (sp-declare-events events 1)
  (sp-initialize 1 2 48000)
  (status-require (example-event (* sp-rate 2) &event))
  (sp-events-add events event)
  (status-require (sp-render-quick events 1))
  (label exit (return status.id)))