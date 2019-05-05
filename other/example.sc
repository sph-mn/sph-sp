(pre-include "sph-sp.h")
(pre-define song-duration 96000)

(define (wvl-path-new duration out) (status-t sp-count-t sp-count-t**)
  status-declare
  (declare
    i spline-path-time-t
    p spline-path-point-t
    s spline-path-segment-t
    wvl-float sp-sample-t*
    segments (array spline-path-segment-t 4))
  (status-require (sp-samples-new duration &wvl-float))
  (status-require (sp-counts-new duration out))
  (set
    s.interpolator spline-path-i-move
    p.x 0
    p.y 600
    (array-get s.points 0) p
    (array-get segments 0) s
    s.interpolator spline-path-i-line
    p.x 24000
    p.y 1800
    (array-get s.points 0) p
    (array-get segments 1) s
    s.interpolator spline-path-i-line
    p.x 96000
    p.y 2400
    (array-get s.points 0) p
    (array-get segments 2) s
    s.interpolator spline-path-i-constant
    (array-get segments 3) s)
  (status-id-require (spline-path-new-get 4 segments 0 duration wvl-float))
  (for ((set i 0) (< i duration) (set i (+ 1 i)))
    (set (array-get *out i) (sp-cheap-round-positive (array-get wvl-float i))))
  (label exit
    (return status)))

(define (wvl2-path-new duration out) (status-t sp-count-t sp-count-t**)
  status-declare
  (declare
    i spline-path-time-t
    p spline-path-point-t
    s spline-path-segment-t
    wvl-float sp-sample-t*
    segments (array spline-path-segment-t 4))
  (status-require (sp-samples-new duration &wvl-float))
  (status-require (sp-counts-new duration out))
  (set
    s.interpolator spline-path-i-move
    p.x 0
    p.y 800
    (array-get s.points 0) p
    (array-get segments 0) s
    s.interpolator spline-path-i-line
    p.x 24000
    p.y 3000
    (array-get s.points 0) p
    (array-get segments 1) s
    s.interpolator spline-path-i-line
    p.x 96000
    p.y 10000
    (array-get s.points 0) p
    (array-get segments 2) s
    s.interpolator spline-path-i-constant
    (array-get segments 3) s)
  (status-id-require (spline-path-new-get 4 segments 0 duration wvl-float))
  (for ((set i 0) (< i duration) (set i (+ 1 i)))
    (set (array-get *out i) (sp-cheap-round-positive (array-get wvl-float i))))
  (label exit
    (return status)))

(define (amp-path-new duration out) (status-t sp-count-t sp-sample-t**)
  status-declare
  (declare
    i spline-path-time-t
    p spline-path-point-t
    s spline-path-segment-t
    segments (array spline-path-segment-t 4))
  (status-require (sp-samples-new duration out))
  (set
    s.interpolator spline-path-i-move
    p.x 0
    p.y 0.5
    (array-get s.points 0) p
    (array-get segments 0) s
    s.interpolator spline-path-i-line
    p.x 24000
    p.y 0.2
    (array-get s.points 0) p
    (array-get segments 1) s
    s.interpolator spline-path-i-line
    p.x 96000
    p.y 0
    (array-get s.points 0) p
    (array-get segments 2) s
    s.interpolator spline-path-i-constant
    (array-get segments 3) s)
  (status-id-require (spline-path-new-get 4 segments 0 duration *out))
  (label exit
    (return status)))

(define (main) int
  status-declare
  (declare
    out sp-block-t
    wvl sp-count-t*
    wvl2 sp-count-t*
    amp sp-sample-t*)
  (declare
    prt sp-synth-partial-t
    events-size sp-count-t
    e sp-event-t
    events (array sp-event-t 10)
    config (array sp-synth-partial-t 10)
    state sp-count-t*)
  (declare
    file sp-file-t
    written sp-count-t)
  (sp-initialise)
  (set events-size 1)
  (sc-comment "paths")
  (status-require (wvl-path-new song-duration &wvl))
  (status-require (wvl2-path-new song-duration &wvl2))
  (status-require (amp-path-new song-duration &amp))
  ;(sp-plot-counts wvl song-duration)
  ;(sp-plot-samples amp song-duration)
  (sc-comment "synth")
  (set (array-get config 0) (sp-synth-partial-1 0 song-duration 0 amp wvl 0))
  (set (array-get config 1) (sp-synth-partial-1 10000 song-duration 0 amp wvl2 0))
  (status-require (sp-synth-event 0 song-duration 1 2 config events))
  (sp-seq-events-prepare events events-size)
  (sc-comment "seq")
  (status-require (sp-block-new 1 song-duration &out))
  (sp-seq 0 song-duration out 0 events events-size)
  (sp-plot-samples *out.samples song-duration)
  ;(status-require (sp-file-open "/tmp/sp.wav" sp-file-mode-write 1 96000 &file))
  ;(status-require (sp-file-write &file out.samples out.size &written))
  ;(printf "wrote %lu\n" written)
  ;(sp-file-close &file)
  (label exit
    (return status.id)))