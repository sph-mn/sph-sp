(sc-comment
  "small example that shows how to use the core sound generators and sc macros."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a series of bursts of noise."
  "see exe/run-example for how to compile and run with gcc")

(sc-include "/usr/share/sph-sp/sc-macros")
(pre-include "sph-sp.h")
(pre-define _rate 48000)

(sp-define-event* noise
  (declare amod sp-sample-t* duration sp-time-t config sp-noise-event-config-t*)
  (set duration (- _event:end _event:start)) (srq (sp-event-memory-init _event 2))
  (srq (sp-noise-event-config-new &config)) (sp-event-memory-add1 _event config)
  (sp-path-samples* &amod duration (line duration _event:volume)) (sp-event-memory-add1 _event amod)
  (sp-noise* _event config (amod amod amp 1 cuth 0.5)) (srq (_event:prepare _event)))

(sp-define-event* (riff (* 2 _rate))
  (declare
    times (array sp-time-t 3 0 (* 1 sp-rate) (* 1.5 sp-rate))
    durations (array sp-time-t 3 (* 0.5 sp-rate) (* 0.15 sp-rate) (* 0.35 sp-rate))
    volumes (array sp-sample-t 3 1.0 0.5 0.75))
  (for-each-index i 3
    (srq
      (sp-group-add-set _event (array-get times i)
        (array-get durations i) (array-get volumes i) 0 noise)))
  (srq (sp-group-prepare _event)))

(define (main) int
  status-declare
  (sp-declare-group song)
  (srq (sp-initialize 1 2 _rate))
  (srq (sp-group-append &song riff))
  (srq (sp-group-append &song riff))
  (srq (sp-group-append &song riff))
  (srq (sp-render-quick song 1))
  (label exit (return status.id)))