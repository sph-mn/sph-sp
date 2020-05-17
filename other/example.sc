(sc-comment
  "creates few seconds long wav file with a synthesised bassdrum."
  "an example for using the synthesiser and sequence output using event groups."
  "can be compiled with: gcc -lsph-sp example.c -o example.exe")

(pre-include "stdio.h" "sph-sp.h")

(define (g1-new start rate out) (status-t sp-time-t sp-time-t sp-event-t*)
  status-declare
  (declare
    b1 sp-event-t
    b1-duration sp-time-t
    b1-partials (array sp-synth-partial-t 1)
    b1-p1-amp sp-sample-t*
    b1-p1-wvl sp-time-t*)
  (sp-group-declare g)
  (status-require (sp-group-new 0 2 4 &g))
  (set b1-duration (rt 1 2))
  (status-require
    (sp-path-samples-3 &b1-p1-amp b1-duration
      (sp-path-line (rt 1 8) 1) (sp-path-line (rt 3 8) 0) (sp-path-line b1-duration 0)))
  (sp-group-memory-add g b1-p1-amp)
  (status-require
    (sp-path-times-3 &b1-p1-wvl b1-duration
      (sp-path-line (rt 1 8) (rt 1 128)) (sp-path-line (rt 2 8) (rt 1 64))
      (sp-path-line b1-duration (rt 1 16))))
  (sp-group-memory-add g b1-p1-wvl)
  (set (array-get b1-partials 0)
    (sp-synth-partial-2 0 b1-duration 0 b1-p1-amp b1-p1-amp b1-p1-wvl b1-p1-wvl 0 0))
  (status-require (sp-synth-event 0 b1-duration 2 1 b1-partials &b1))
  (sp-group-add g b1)
  (sp-group-prepare g)
  (set *out g)
  (label exit (if status-is-failure (sp-group-free g)) status-return))

(define (song-new rate out) (status-t sp-time-t sp-event-t*)
  status-declare
  (declare e sp-event-t)
  (sp-group-declare song)
  (status-require (sp-group-new 0 2 0 &song))
  (status-require (g1-new 0 rate &e))
  (sp-group-append &song e)
  (status-require (g1-new 0 rate &e))
  (sp-group-append &song e)
  (sp-group-prepare song)
  (set *out song)
  (label exit (return status)))

(define (main) int
  status-declare
  (sp-initialise 1)
  (declare song sp-event-t rate sp-time-t)
  (declare-render-config render-config)
  (set rate render-config.rate)
  (status-require (song-new rate &song))
  (status-require (sp-render-file song 0 (rt 3 1) render-config "/tmp/song.wav"))
  (song.free &song)
  (label exit (printf "status: %d\n" status.id) (return status.id)))