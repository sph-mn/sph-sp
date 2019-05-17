(sc-comment
  "# compilation
   scc sp.sc -- -o sp.bin -lsph-sp &&
   chmod +x sp.bin &&
   ./sp.bin")

(pre-include "sph-sp.h")
(pre-define song-duration 960000)

(define (wvl-path-new duration out) (status-t sp-count-t sp-count-t**)
  status-declare
  (declare wvl-float sp-sample-t*)
  (status-require (sp-samples-new duration &wvl-float))
  (status-require (sp-counts-new duration out))
  (spline-path-new-get-4 wvl-float duration
    (spline-path-move 0 600) (spline-path-line 24000 1800) (spline-path-line duration 2400)
    (spline-path-constant))
  (sp-counts-from-samples wvl-float duration *out)
  (label exit (return status)))

(define (wvl2-path-new duration out) (status-t sp-count-t sp-count-t**)
  status-declare
  (declare wvl-float sp-sample-t*)
  (status-require (sp-samples-new duration &wvl-float))
  (status-require (sp-counts-new duration out))
  (spline-path-new-get-4 wvl-float duration
    (spline-path-move 0 20) (spline-path-line 24000 10) (spline-path-line duration 40)
    (spline-path-constant))
  (sp-counts-from-samples wvl-float duration *out)
  (label exit (return status)))

(define (amp-path-new duration out) (status-t sp-count-t sp-sample-t**)
  status-declare
  (status-require (sp-samples-new duration out))
  (status-id-require
    (spline-path-new-get-4 *out duration
      (spline-path-move 0 1) (spline-path-line 24000 0.3) (spline-path-line duration 0)
      (spline-path-constant)))
  (label exit (return status)))

(define (constant-path-new value duration out) (status-t sp-sample-t sp-count-t sp-sample-t**)
  status-declare
  (status-require (sp-samples-new duration out))
  (status-id-require
    (spline-path-new-get-2 *out duration (spline-path-move 0 value) (spline-path-constant)))
  (label exit (return status)))

#;(define (noise) void
    (set *amp amp1)
  (status-require
    (sp-noise-event
      0 song-duration amp cut-l cut-h trn-l trn-h #f #f sp-default-random-state events))
  (status-require
    (sp-cheap-noise-event-lp 0 song-duration amp cut-l 2 0 0 sp-default-random-state events)))

(define (main) int
  status-declare
  (declare
    out sp-block-t
    wvl sp-count-t*
    wvl2 sp-count-t*
    cut-l sp-sample-t*
    cut-h sp-sample-t*
    trn-l sp-sample-t*
    trn-h sp-sample-t*
    amp1 sp-sample-t*
    amp2 sp-sample-t*
    amp (array sp-sample-t* sp-channel-limit))
  (declare
    prt sp-synth-partial-t
    events-size sp-count-t
    e sp-event-t
    events (array sp-event-t 10)
    config (array sp-synth-partial-t 10)
    state sp-count-t*)
  (sp-initialise 0)
  (status-require (wvl-path-new song-duration &wvl))
  (status-require (wvl2-path-new song-duration &wvl2))
  (status-require (amp-path-new song-duration &amp1))
  (status-require (constant-path-new 0.01 song-duration &cut-l))
  (status-require (constant-path-new 0.1 song-duration &cut-h))
  (status-require (constant-path-new 0.07 song-duration &trn-l))
  (status-require (constant-path-new 0.5 song-duration &amp2))
  (set trn-h trn-l)
  (array-set config
    0 (sp-synth-partial-1 0 song-duration 0 amp1 wvl 0)
    1 (sp-synth-partial-1 0 song-duration 1 amp2 wvl2 0))
  (status-require (sp-synth-event 0 song-duration 1 2 config events))
  (set events-size 1)
  (sp-seq-events-prepare events events-size)
  (status-require (sp-block-new 1 song-duration &out))
  (sp-seq 0 song-duration out 0 events events-size)
  ;(sp-plot-samples *out.samples song-duration)
  (sp-block->file out "/tmp/sp.wav" 96000)
  (sp-events-free events events-size)
  (label exit (return status.id)))