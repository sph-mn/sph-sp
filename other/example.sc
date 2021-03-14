(sc-comment
  "small example that shows how to use the core sound generators and sc macros."
  "this example depends on gnuplot to be installed."
  "when running it, it should display a gnuplot window with a series of bursts of noise."
  "see exe/run-example for how to compile and run with gcc")

(sc-include "/usr/share/sph-sp/sc-macros")
(pre-include "sph-sp.h")
(pre-define _rate 48000)

(sp-define-event* noise
  (sp-event-memory* 2)
  (sp-path-samples* amod line (_duration _event:volume))
  (sp-event-memory-add1 _event amod)
  (sp-noise-config* config)
  (sp-noise* _event config (amod amod amp 1 cuth 0.5)))

(sp-define-group* (riff (* 2 _rate))
  (sp-times* times 0 (* 1 sp-rate) (* 1.5 sp-rate))
  (sp-times* durations (* 0.5 sp-rate) (* 0.15 sp-rate) (* 0.35 sp-rate))
  (sp-samples* volumes 1.0 0.5 0.75)
  (for-each-index i 3
    (srq
      (sp-group-add-set _event (array-get times i)
        (array-get durations i) (array-get volumes i) 0 noise))))

(define (main) int
  status-declare
  (sp-declare-group song)
  (srq (sp-initialize 1 2 _rate))
  (srq (sp-group-append &song riff))
  (srq (sp-group-append &song riff))
  (srq (sp-group-append &song riff))
  (srq (sp-render-quick song 1))
  (label exit (return status.id)))