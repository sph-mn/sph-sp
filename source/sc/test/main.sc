(pre-include "./helper.c")

(pre-cond
  ( (= sp-sample-format-f64 sp-sample-format)
    (pre-define
      sp-sample-nearly-equal f64-nearly-equal
      sp-sample-array-nearly-equal f64-array-nearly-equal))
  ( (= sp-sample-format-f32 sp-sample-format)
    (pre-define
      sp-sample-nearly-equal f32-nearly-equal
      sp-sample-array-nearly-equal f32-array-nearly-equal)))

(define error-margin sp-sample-t 0.1)
(define test-file-path uint8-t* "/tmp/test-sph-sp-file")

(define (test-base) status-t
  status-declare
  (test-helper-assert "input 0.5" (sp-sample-nearly-equal 0.63662 (sp-sinc 0.5) error-margin))
  (test-helper-assert "input 1" (sp-sample-nearly-equal 1.0 (sp-sinc 0) error-margin))
  (test-helper-assert "window-blackman 0 51"
    (sp-sample-nearly-equal 0 (sp-window-blackman 0 51) error-margin))
  (test-helper-assert "window-blackman 25 51"
    (sp-sample-nearly-equal 1 (sp-window-blackman 25 51) error-margin))
  (label exit status-return))

(define (test-spectral-inversion-ir) status-t
  status-declare
  (declare a-len sp-time-t a (array sp-sample-t 5 0.1 -0.2 0.3 -0.2 0.1))
  (set a-len 5)
  (sp-spectral-inversion-ir a a-len)
  (test-helper-assert "result check"
    (and (sp-sample-nearly-equal -0.1 (array-get a 0) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 1) error-margin)
      (sp-sample-nearly-equal 0.7 (array-get a 2) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 3) error-margin)
      (sp-sample-nearly-equal -0.1 (array-get a 4) error-margin)))
  (label exit status-return))

(define (test-spectral-reversal-ir) status-t
  status-declare
  (declare a-len sp-time-t a (array sp-sample-t 5 0.1 -0.2 0.3 -0.2 0.1))
  (set a-len 5)
  (sp-spectral-reversal-ir a a-len)
  (test-helper-assert "result check"
    (and (sp-sample-nearly-equal 0.1 (array-get a 0) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 1) error-margin)
      (sp-sample-nearly-equal 0.3 (array-get a 2) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 3) error-margin)
      (sp-sample-nearly-equal 0.1 (array-get a 4) error-margin)))
  (label exit status-return))

(define (test-convolve) status-t
  status-declare
  (declare
    a sp-sample-t*
    a-len sp-time-t
    b sp-sample-t*
    b-len sp-time-t
    carryover sp-sample-t*
    carryover-len sp-time-t
    result sp-sample-t*
    result-len sp-time-t
    sample-count sp-time-t
    expected-result (array sp-sample-t (5) 2 7 16 22 28)
    expected-carryover (array sp-sample-t (3) 27 18 0))
  (memreg-init 4)
  (set sample-count 5 b-len 3 result-len sample-count a-len sample-count carryover-len b-len)
  (status-require (sph-helper-calloc (* result-len (sizeof sp-sample-t)) &result))
  (memreg-add result)
  (status-require (sph-helper-calloc (* a-len (sizeof sp-sample-t)) &a))
  (memreg-add a)
  (status-require (sph-helper-calloc (* b-len (sizeof sp-sample-t)) &b))
  (memreg-add b)
  (status-require (sph-helper-calloc (* carryover-len (sizeof sp-sample-t)) &carryover))
  (memreg-add carryover)
  (sc-comment "prepare input/output data arrays")
  (array-set a 0 2 1 3 2 4 3 5 4 6)
  (array-set b 0 1 1 2 2 3)
  (array-set carryover 0 0 1 0 2 0)
  (sc-comment "test convolve first segment")
  (sp-convolve a a-len b b-len carryover-len carryover result)
  (test-helper-assert "first result"
    (sp-sample-array-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert "first result carryover"
    (sp-sample-array-nearly-equal carryover carryover-len
      expected-carryover carryover-len error-margin))
  (sc-comment "test convolve second segment")
  (array-set a 0 8 1 9 2 10 3 11 4 12)
  (array-set expected-result 0 35 1 43 2 52 3 58 4 64)
  (array-set expected-carryover 0 57 1 36 2 0)
  (sp-convolve a a-len b b-len carryover-len carryover result)
  (test-helper-assert "second result"
    (sp-sample-array-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert "second result carryover"
    (sp-sample-array-nearly-equal carryover carryover-len
      expected-carryover carryover-len error-margin))
  (label exit memreg-free status-return))

(define (test-moving-average) status-t
  status-declare
  (declare
    radius sp-time-t
    out (array sp-sample-t (5) 0 0 0 0 0)
    in (array sp-sample-t (5) 1 3 5 7 8)
    prev (array sp-sample-t (5) 9 10 11)
    next (array sp-sample-t (5) 12 13 14)
    in-end sp-sample-t*
    prev-end sp-sample-t*
    next-end sp-sample-t*
    in-window sp-sample-t*
    in-window-end sp-sample-t*)
  (set
    prev-end (+ prev 2)
    next-end (+ next 2)
    in-end (+ in 4)
    in-window (+ in 1)
    in-window-end (+ in 3)
    radius 3)
  (status-require
    (sp-moving-average in in-end in-window in-window-end prev prev-end next next-end radius out))
  (sc-comment "first run with prev and next and only index 1 to 3 inclusively processed")
  (test-helper-assert "moving-average 1.1"
    (sp-sample-nearly-equal 6.142857142857143 (array-get out 0) error-margin))
  (test-helper-assert "moving-average 1.2"
    (sp-sample-nearly-equal 6.571428571428571 (array-get out 1) error-margin))
  (test-helper-assert "moving-average 1.2"
    (sp-sample-nearly-equal 7 (array-get out 2) error-margin))
  (sc-comment "second run. result number series will be symmetric")
  (array-set out 0 0 1 0 2 0 3 0 4 0)
  (array-set in 0 2 1 2 2 2 3 2 4 2)
  (status-require (sp-moving-average in in-end in in-end 0 0 0 0 1 out))
  (test-helper-assert "moving-average 2.1"
    (sp-sample-nearly-equal 1.3 (array-get out 0) error-margin))
  (test-helper-assert "moving-average 2.2"
    (sp-sample-nearly-equal 2 (array-get out 1) error-margin))
  (test-helper-assert "moving-average 2.3"
    (sp-sample-nearly-equal 2 (array-get out 2) error-margin))
  (test-helper-assert "moving-average 2.4"
    (sp-sample-nearly-equal 2 (array-get out 3) error-margin))
  (test-helper-assert "moving-average 2.5"
    (sp-sample-nearly-equal 1.3 (array-get out 4) error-margin))
  (label exit status-return))

(define (test-windowed-sinc) status-t
  status-declare
  (declare
    transition sp-float-t
    cutoff sp-float-t
    ir sp-sample-t*
    ir-len sp-time-t
    state sp-convolution-filter-state-t*
    source (array sp-sample-t 10 3 4 5 6 7 8 9 0 1 2)
    result (array sp-sample-t 10 0 0 0 0 0 0 0 0 0 0))
  (set state 0 cutoff 0.1 transition 0.08)
  (sc-comment "ir functions")
  (status-require (sp-windowed-sinc-lp-hp-ir cutoff transition #f &ir &ir-len))
  (test-helper-assert "ir" (sp-sample-nearly-equal 0.0952 (array-get ir 28) error-margin))
  (status-require (sp-windowed-sinc-bp-br-ir 0.1 0.4 0.08 0.08 #f &ir &ir-len))
  (status-require (sp-windowed-sinc-lp-hp-ir cutoff transition #t &ir &ir-len))
  (status-require (sp-windowed-sinc-bp-br-ir cutoff cutoff transition transition #t &ir &ir-len))
  (sc-comment "filter functions")
  (status-require (sp-windowed-sinc-lp-hp source 10 0.1 0.08 #f &state result))
  (sp-convolution-filter-state-free state)
  (set state 0)
  (status-require (sp-windowed-sinc-lp-hp source 10 0.1 0.08 #t &state result))
  (sp-convolution-filter-state-free state)
  (set state 0)
  (status-require (sp-windowed-sinc-bp-br source 10 0.1 0.4 0.08 0.08 #f &state result))
  (sp-convolution-filter-state-free state)
  (set state 0)
  (status-require (sp-windowed-sinc-bp-br source 10 0.1 0.4 0.08 0.08 #t &state result))
  (sp-convolution-filter-state-free state)
  (label exit status-return))

(define (test-file) status-t
  status-declare
  (declare
    channel sp-time-t
    channel-count sp-channels-t
    block sp-block-t
    block-2 sp-block-t
    len sp-time-t
    file sp-file-t
    position sp-time-t
    sample-count sp-time-t
    result-sample-count sp-time-t
    sample-rate sp-sample-rate-t
    unequal int8-t)
  (if (file-exists test-file-path) (unlink test-file-path))
  (set channel-count 2 sample-rate 8000 sample-count 5 position 0 channel channel-count)
  (sp-block-zero block)
  (sp-block-zero block-2)
  (status-require (sp-block-new channel-count sample-count &block))
  (status-require (sp-block-new channel-count sample-count &block-2))
  (while channel
    (set channel (- channel 1) len sample-count)
    (while len (set len (- len 1) (array-get (array-get block.samples channel) len) len)))
  (sc-comment "test create")
  (status-require
    (sp-file-open test-file-path sp-file-mode-read-write channel-count sample-rate &file))
  (printf "  create\n")
  (status-require (sp-file-position &file &position))
  (status-require (sp-file-write &file block.samples sample-count &result-sample-count))
  (status-require (sp-file-position &file &position))
  (test-helper-assert "sp-file-position file after write" (= sample-count position))
  (status-require (sp-file-position-set &file 0))
  (status-require (sp-file-read &file sample-count block-2.samples &result-sample-count))
  (sc-comment "compare read result with output data")
  (set len channel-count unequal 0)
  (while (and len (not unequal))
    (set
      len (- len 1)
      unequal
      (not
        (sp-sample-array-nearly-equal (array-get block.samples len) sample-count
          (array-get block-2.samples len) sample-count error-margin))))
  (test-helper-assert "sp-file-read new file result" (not unequal))
  (status-require (sp-file-close &file))
  (printf "  write\n")
  (sc-comment "test open")
  (status-require (sp-file-open test-file-path sp-file-mode-read-write 2 8000 &file))
  (status-require (sp-file-position &file &position))
  (test-helper-assert "sp-file-position existing file" (= sample-count position))
  (status-require (sp-file-position-set &file 0))
  (sp-file-read &file sample-count block-2.samples &result-sample-count)
  (sc-comment "compare read result with output data")
  (set unequal 0 len channel-count)
  (while (and len (not unequal))
    (set
      len (- len 1)
      unequal
      (not
        (sp-sample-array-nearly-equal (array-get block.samples len) sample-count
          (array-get block-2.samples len) sample-count error-margin))))
  (test-helper-assert "sp-file-read existing result" (not unequal))
  (status-require (sp-file-close &file))
  (printf "  open\n")
  (label exit (sp-block-free block) (sp-block-free block-2) status-return))

(define (test-fft) status-t
  status-declare
  (declare
    a-real (array sp-sample-t 6 -0.6 0.1 0.4 0.8 0 0)
    a-imag (array sp-sample-t 6 0 0 0 0 0 0)
    a-len sp-time-t)
  (set a-len 6)
  (status-i-require (sp-fft a-len a-real a-imag))
  (status-i-require (sp-ffti a-len a-real a-imag))
  (label exit status-return))

(define (test-synth) status-t
  status-declare
  (declare
    state sp-time-t*
    config-len sp-time-t
    out1 sp-block-t
    out2 sp-block-t
    channels sp-channels-t
    duration sp-time-t
    prt1 sp-synth-partial-t
    prt2 sp-synth-partial-t
    prt3 sp-synth-partial-t
    config (array sp-synth-partial-t 3)
    wvl (array sp-time-t 4 2 2 2 2)
    amp (array sp-sample-t 4 0.1 0.2 0.3 0.4))
  (set
    state 0
    duration 4
    channels 2
    config-len 3
    prt1.modifies 0
    prt2.modifies 1
    prt3.modifies 0
    prt1.start 0
    prt2.start prt1.start
    prt3.start prt1.start
    prt1.end duration
    prt2.end prt1.end
    prt3.end prt1.end
    (array-get prt1.amp 0) amp
    (array-get prt1.amp 1) amp
    (array-get prt1.wvl 0) wvl
    (array-get prt1.wvl 1) wvl
    (array-get prt1.phs 0) 1
    (array-get prt1.phs 2) 2
    (array-get prt2.amp 0) amp
    (array-get prt2.amp 1) amp
    (array-get prt2.wvl 0) wvl
    (array-get prt2.wvl 1) wvl
    (array-get prt2.phs 0) 1
    (array-get prt2.phs 2) 2
    (array-get prt3.amp 0) amp
    (array-get prt3.amp 1) amp
    (array-get prt3.wvl 0) wvl
    (array-get prt3.wvl 1) wvl
    (array-get prt3.phs 0) 1
    (array-get prt3.phs 2) 2
    (array-get config 0) prt1
    (array-get config 1) prt2
    (array-get config 2) prt3)
  (status-require (sp-block-new channels duration &out1))
  (status-require (sp-block-new channels duration &out2))
  (status-require (sp-synth-state-new channels config-len config &state))
  (status-require (sp-synth out1 0 duration config-len config state))
  (status-require (sp-synth out2 0 duration config-len config state))
  (sp-block-free out1)
  (sp-block-free out2)
  (label exit status-return))

(define (test-sp-plot) status-t
  "better test separately as it opens gnuplot windows"
  status-declare
  (declare a (array sp-sample-t 9 0.1 -0.2 0.1 -0.4 0.3 -0.4 0.2 -0.2 0.1))
  (sp-plot-samples a 9)
  (sp-plot-spectrum a 9)
  (label exit status-return))

(define (test-sp-triangle-square) status-t
  status-declare
  (declare i sp-time-t out-t sp-sample-t* out-s sp-sample-t*)
  (status-require (sph-helper-calloc (* 96000 (sizeof sp-sample-t*)) &out-t))
  (status-require (sph-helper-calloc (* 96000 (sizeof sp-sample-t*)) &out-s))
  (for ((set i 0) (< i 96000) (set i (+ 1 i)))
    (set (array-get out-t i) (sp-triangle-96 i) (array-get out-s i) (sp-square-96 i)))
  (test-helper-assert "triangle 0" (= 0 (array-get out-t 0)))
  (test-helper-assert "triangle 1/2" (= 1 (array-get out-t 48000)))
  (test-helper-assert "triangle 1" (f64-nearly-equal 0 (array-get out-t 95999) error-margin))
  (test-helper-assert "square 0" (= -1 (array-get out-s 0)))
  (test-helper-assert "square 1/4" (= -1 (array-get out-s 24000)))
  (test-helper-assert "square 1/2 - 1" (= -1 (array-get out-s 47999)))
  (test-helper-assert "square 1/2" (= 1 (array-get out-s 48000)))
  (test-helper-assert "square 3/4" (= 1 (array-get out-s 72000)))
  (test-helper-assert "square 1" (= 1 (array-get out-s 95999)))
  (free out-t)
  (free out-s)
  (label exit status-return))

(define (test-sp-random) status-t
  status-declare
  (declare s sp-random-state-t out (array sp-sample-t 20))
  (set s (sp-random-state-new 80))
  (sp-random-samples &s 10 out)
  (sp-random-samples &s 10 (+ 10 out))
  (test-helper-assert "last value" (f64-nearly-equal 0.355602 (array-get out 19) error-margin))
  (label exit status-return))

(pre-define (max a b) (if* (> a b) a b) (min a b) (if* (< a b) a b))
(pre-define sp-noise-duration 96)

(define (test-sp-noise-event) status-t
  status-declare
  (declare
    events (array sp-event-t 1)
    events-size sp-time-t
    out sp-block-t
    cut-l (array sp-sample-t sp-noise-duration)
    cut-h (array sp-sample-t sp-noise-duration)
    trn-l (array sp-sample-t sp-noise-duration)
    trn-h (array sp-sample-t sp-noise-duration)
    amp1 (array sp-sample-t sp-noise-duration)
    amp (array sp-sample-t* sp-channel-limit)
    i sp-time-t)
  (status-require (sp-block-new 1 sp-noise-duration &out))
  (set (array-get amp 0) amp1)
  (for ((set i 0) (< i sp-noise-duration) (set i (+ 1 i)))
    (set
      (array-get cut-l i) (if* (< i (/ sp-noise-duration 2)) 0.01 0.1)
      (array-get cut-h i) 0.11
      (array-get trn-l i) 0.07
      (array-get trn-h i) 0.07
      (array-get amp1 i) 1.0))
  (status-require
    (sp-noise-event 0 sp-noise-duration
      amp cut-l cut-h trn-l trn-h #f 30 sp-default-random-state events))
  (set events-size 1)
  (sp-seq 0 sp-noise-duration out events events-size)
  (sp-events-free events events-size)
  (sp-block-free out)
  (label exit status-return))

(define (test-sp-cheap-filter) status-t
  status-declare
  (declare
    state sp-cheap-filter-state-t
    out (array sp-sample-t sp-noise-duration)
    in (array sp-sample-t sp-noise-duration)
    i sp-time-t
    s sp-random-state-t)
  (set s (sp-random-state-new 80))
  (sp-random-samples &s sp-noise-duration in)
  (status-require (sp-cheap-filter-state-new sp-noise-duration sp-cheap-filter-passes-limit &state))
  (sp-cheap-filter-lp in sp-noise-duration 0.2 1 0 1 &state out)
  (sp-cheap-filter-lp in sp-noise-duration 0.2 sp-cheap-filter-passes-limit 0 1 &state out)
  (sp-cheap-filter-lp in sp-noise-duration 0.2 sp-cheap-filter-passes-limit 0 1 &state out)
  (sp-cheap-filter-state-free &state)
  (label exit status-return))

(define (test-sp-cheap-noise-event) status-t
  status-declare
  (declare
    events (array sp-event-t 1)
    events-size sp-time-t
    out sp-block-t
    cut (array sp-sample-t sp-noise-duration)
    amp1 (array sp-sample-t sp-noise-duration)
    amp (array sp-sample-t* sp-channel-limit)
    q-factor sp-sample-t
    i sp-time-t)
  (status-require (sp-block-new 1 sp-noise-duration &out))
  (set (array-get amp 0) amp1 q-factor 0)
  (for ((set i 0) (< i sp-noise-duration) (set i (+ 1 i)))
    (set
      (array-get cut i) (if* (< i (/ sp-noise-duration 2)) 0.01 0.1)
      (array-get cut i) 0.08
      (array-get amp1 i) 1.0))
  (status-require
    (sp-cheap-noise-event-lp 0 sp-noise-duration amp cut 1 0 #f sp-default-random-state events))
  (set events-size 1)
  (sp-seq 0 sp-noise-duration out events events-size)
  (sp-events-free events events-size)
  (sp-block-free out)
  (label exit status-return))

(pre-define sp-seq-event-count 2)

(define (test-sp-seq) status-t
  status-declare
  (declare events (array sp-event-t sp-seq-event-count) out sp-block-t i sp-time-t)
  (set
    (array-get events 0) (test-helper-event 0 40 1)
    (array-get events 1) (test-helper-event 41 100 2))
  (sp-seq-events-prepare events sp-seq-event-count)
  (status-require (sp-block-new 2 100 &out))
  (sp-seq 0 50 out events sp-seq-event-count)
  (sp-seq 50 100 (sp-block-with-offset out 50) events sp-seq-event-count)
  (test-helper-assert "block contents 1 event 1"
    (and (= 1 (array-get out.samples 0 0)) (= 1 (array-get out.samples 0 39))))
  (test-helper-assert "block contents 1 gap" (= 0 (array-get out.samples 0 40)))
  (test-helper-assert "block contents 1 event 2"
    (and (= 2 (array-get out.samples 0 41)) (= 2 (array-get out.samples 0 99))))
  (sc-comment "sp-seq-parallel")
  (status-require (sp-seq-parallel 0 100 out events sp-seq-event-count))
  (sp-events-free events sp-seq-event-count)
  (sp-block-free out)
  (label exit status-return))

(define (test-sp-group) status-t
  status-declare
  (declare
    g sp-event-t
    g1 sp-event-t
    e1 sp-event-t
    e2 sp-event-t
    e3 sp-event-t
    block sp-block-t
    m1 sp-time-t*
    m2 sp-time-t*)
  (status-require (sp-times-new 100 &m1))
  (status-require (sp-times-new 100 &m2))
  (status-require (sp-group-new 0 2 2 &g))
  (status-require (sp-group-new 10 2 0 &g1))
  (status-require (sp-block-new 2 100 &block))
  (set e1 (test-helper-event 0 20 1) e2 (test-helper-event 20 40 2) e3 (test-helper-event 50 100 3))
  (sp-group-add g1 e1)
  (sp-group-add g1 e2)
  (sp-group-add g g1)
  (sp-group-add g e3)
  (sp-group-memory-add g m1)
  (sp-group-memory-add g m2)
  (sp-group-prepare g)
  (g.f 0 50 block &g)
  (g.f 50 100 (sp-block-with-offset block 50) &g)
  (g.free &g)
  (test-helper-assert "block contents event 1"
    (and (= 1 (array-get block.samples 0 10)) (= 1 (array-get block.samples 0 29))))
  (test-helper-assert "block contents event 2"
    (and (= 2 (array-get block.samples 0 30)) (= 2 (array-get block.samples 0 39))))
  (test-helper-assert "block contents gap"
    (and (> 1 (array-get block.samples 0 40)) (> 1 (array-get block.samples 0 49))))
  (test-helper-assert "block contents event 3"
    (and (= 3 (array-get block.samples 0 50)) (= 3 (array-get block.samples 0 99))))
  (sp-block-free block)
  (label exit status-return))

(pre-define sp-synth-event-duration 100 sp-synth-event-half-duration (/ sp-synth-event-duration 2))

(define (test-sp-synth-event) status-t
  "sp synth values were taken from printing index and value of the result array.
   sp-plot-samples can plot the result"
  status-declare
  (declare
    event sp-event-t
    out sp-block-t
    wvl1 (array sp-time-t sp-synth-event-duration)
    wvl2 (array sp-time-t sp-synth-event-duration)
    amp1 (array sp-sample-t sp-synth-event-duration)
    amp2 (array sp-sample-t sp-synth-event-duration)
    partials (array sp-synth-partial-t 2)
    i sp-time-t
    ci sp-channels-t)
  (for ((set i 0) (< i sp-synth-event-duration) (set+ i 1))
    (set (array-get wvl1 i) 5 (array-get wvl2 i) 10 (array-get amp1 i) 0.1 (array-get amp2 i) 1))
  (set
    (array-get partials 0)
    (sp-synth-partial-2 0 sp-synth-event-half-duration 0 amp1 amp2 wvl1 wvl2 0 0)
    (array-get partials 1)
    (sp-synth-partial-2 sp-synth-event-half-duration sp-synth-event-duration
      0 amp2 amp1 wvl2 wvl1 0 0))
  (status-require (sp-synth-event 0 sp-synth-event-duration 2 2 partials &event))
  (status-require (sp-block-new 2 sp-synth-event-duration &out))
  (event.f 0 sp-synth-event-duration out &event)
  (test-helper-assert "values 1" (>= 0.1 (array-get out.samples 0 0)))
  (test-helper-assert "values 2" (= 0 (array-get out.samples 0 49)))
  (test-helper-assert "values 3" (> 0.1 (array-get out.samples 0 44)))
  (test-helper-assert "values 4" (<= 0.1 (array-get out.samples 0 50)))
  (test-helper-assert "values 5" (= 1 (array-get out.samples 0 54)))
  (test-helper-assert "values 6" (> 0.1 (array-get out.samples 0 99)))
  (sp-block-free out)
  (event.free &event)
  (label exit status-return))

(define (main) int
  "\"goto exit\" can skip events"
  status-declare
  (sp-initialise 3)
  (test-helper-test-one test-sp-group)
  (test-helper-test-one test-sp-synth-event)
  (test-helper-test-one test-sp-seq)
  (test-helper-test-one test-sp-cheap-noise-event)
  (test-helper-test-one test-sp-cheap-filter)
  (test-helper-test-one test-sp-noise-event)
  (test-helper-test-one test-sp-random)
  (test-helper-test-one test-sp-triangle-square)
  (test-helper-test-one test-synth)
  (test-helper-test-one test-moving-average)
  (test-helper-test-one test-fft)
  (test-helper-test-one test-spectral-inversion-ir)
  (test-helper-test-one test-base)
  (test-helper-test-one test-spectral-reversal-ir)
  (test-helper-test-one test-convolve)
  (test-helper-test-one test-file)
  (test-helper-test-one test-windowed-sinc)
  (label exit (test-helper-display-summary) (return status.id)))