(pre-include "./helper.c")
(sc-include "../main/sc-macros")

(pre-cond
  ( (= sp-sample-format-f64 sp-sample-format)
    (pre-define
      sp-sample-nearly-equal f64-nearly-equal
      sp-samples-nearly-equal f64-array-nearly-equal))
  ( (= sp-sample-format-f32 sp-sample-format)
    (pre-define
      sp-sample-nearly-equal f32-nearly-equal
      sp-samples-nearly-equal f32-array-nearly-equal)))

(pre-define _rate 960)
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
    (sp-samples-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert "first result carryover"
    (sp-samples-nearly-equal carryover carryover-len expected-carryover carryover-len error-margin))
  (sc-comment "test convolve second segment")
  (array-set a 0 8 1 9 2 10 3 11 4 12)
  (array-set expected-result 0 35 1 43 2 52 3 58 4 64)
  (array-set expected-carryover 0 57 1 36 2 0)
  (sp-convolve a a-len b b-len carryover-len carryover result)
  (test-helper-assert "second result"
    (sp-samples-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert "second result carryover"
    (sp-samples-nearly-equal carryover carryover-len expected-carryover carryover-len error-margin))
  (label exit memreg-free status-return))

(define (test-moving-average) status-t
  status-declare
  (declare in sp-sample-t* out sp-sample-t* radius sp-time-t size sp-time-t)
  (memreg-init 2)
  (set size 11 radius 4)
  (status-require (sp-path-samples-2 &in size (sp-path-line 5 10.0) (sp-path-line (- size 1) 0)))
  (memreg-add in)
  (status-require (sp-samples-new size &out))
  (memreg-add out)
  (sc-comment "without prev/next")
  (sp-moving-average in size 0 0 radius out)
  (test-helper-assert "without prev/next"
    (and (sp-sample-nearly-equal 2.0 (array-get out 1) error-margin)
      (sp-sample-nearly-equal 5.5 (array-get out 5) error-margin)
      (sp-sample-nearly-equal 2.0 (array-get out 9) error-margin)))
  (sc-comment "with prev/next")
  (sp-samples-zero out size)
  (declare
    prev (array sp-sample-t 11 0 0 0 0 0 0 0 -8 -6 -4 -1)
    next (array sp-sample-t 4 -1 -4 -6 -8))
  (sp-moving-average in size prev next radius out)
  (test-helper-assert "with prev/next"
    (and (sp-sample-nearly-equal 2.11 (array-get out 1) error-margin)
      (sp-sample-nearly-equal 5.5 (array-get out 5) error-margin)
      (sp-sample-nearly-equal 2.11 (array-get out 9) error-margin)))
  (label exit memreg-free status-return))

(define (test-windowed-sinc) status-t
  status-declare
  (declare
    transition sp-sample-t
    cutoff sp-sample-t
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
    block-2 sp-block-t
    block sp-block-t
    channel-count sp-channel-count-t
    channel sp-time-t
    file sp-file-t
    len sp-time-t
    position sp-time-t
    result-sample-count sp-time-t
    sample-count sp-time-t
    sample-rate sp-time-t
    unequal int8-t)
  (if (file-exists test-file-path) (unlink test-file-path))
  (set channel-count 2 sample-rate 8000 sample-count 5 position 0 channel channel-count)
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
        (sp-samples-nearly-equal (array-get block.samples len) sample-count
          (array-get block-2.samples len) sample-count error-margin))))
  (test-helper-assert "sp-file-read new file result" (not unequal))
  (status-require (sp-file-close file))
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
        (sp-samples-nearly-equal (array-get block.samples len) sample-count
          (array-get block-2.samples len) sample-count error-margin))))
  (test-helper-assert "sp-file-read existing result" (not unequal))
  (status-require (sp-file-close file))
  (printf "  open\n")
  (label exit (sp-block-free &block) (sp-block-free &block-2) status-return))

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

(define (test-sp-plot) status-t
  "better test separately as it opens gnuplot windows"
  status-declare
  (declare a (array sp-sample-t 9 0.1 -0.2 0.1 -0.4 0.3 -0.4 0.2 -0.2 0.1))
  (sp-plot-samples a 9)
  (sp-plot-spectrum a 9)
  (label exit status-return))

(define (test-sp-triangle-square) status-t
  status-declare
  (declare i sp-time-t out-t sp-sample-t* out-s sp-sample-t* size sp-time-t)
  (set size 96000)
  (status-require (sph-helper-calloc (* size (sizeof sp-sample-t*)) &out-t))
  (status-require (sph-helper-calloc (* size (sizeof sp-sample-t*)) &out-s))
  (for ((set i 0) (< i size) (set+ i 1))
    (set
      (array-get out-t i) (sp-triangle i (/ size 2) (/ size 2))
      (array-get out-s i) (sp-square i size)))
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
  (sp-samples-random &s 10 out)
  (sp-samples-random &s 10 (+ 10 out))
  (test-helper-assert "last value" (f64-nearly-equal -0.553401 (array-get out 19) error-margin))
  (label exit status-return))

(pre-define (max a b) (if* (> a b) a b) (min a b) (if* (< a b) a b))
(pre-define test-noise-duration 960)

(define (test-sp-noise-event) status-t
  status-declare
  (declare
    out sp-block-t
    cutl (array sp-sample-t test-noise-duration)
    cuth (array sp-sample-t test-noise-duration)
    trnl (array sp-sample-t test-noise-duration)
    trnh (array sp-sample-t test-noise-duration)
    amod (array sp-sample-t test-noise-duration)
    i sp-time-t
    config sp-noise-event-config-t*)
  (sp-declare-event event)
  (sp-declare-event-list events)
  (free-on-error-init 2)
  (status-require (sp-noise-event-config-new &config))
  (free-on-error1 config)
  (status-require (sp-block-new 2 test-noise-duration &out))
  (free-on-error &out sp-block-free)
  (for-each-index i test-noise-duration
    (set (array-get cutl i) 0.01 (array-get cuth i) 0.3 (array-get amod i) 1.0))
  (struct-set *config cutl-mod cutl cuth-mod cuth amod amod amp 1 channels 2 trnh 0.07 trnl 0.07)
  (struct-set event start 0 end test-noise-duration prepare sp-noise-event-prepare data config)
  (status-require (sp-event-list-add &events event))
  (status-require (sp-seq 0 test-noise-duration out &events))
  (declare sum sp-sample-t)
  (test-helper-assert "in range -1..1"
    (>= 1.0 (sp-samples-absolute-max (array-get out.samples 0) test-noise-duration)))
  (test-helper-assert "value"
    (f64-nearly-equal 0.514491 (array-get (array-get out.samples 0) 10) 0.001))
  (sp-block-free &out)
  (label exit (if status-is-failure free-on-error-free) status-return))

(define (test-sp-cheap-filter) status-t
  status-declare
  (declare
    state sp-cheap-filter-state-t
    out (array sp-sample-t test-noise-duration)
    in (array sp-sample-t test-noise-duration)
    i sp-time-t
    s sp-random-state-t)
  (set s (sp-random-state-new 80))
  (sp-samples-random &s test-noise-duration in)
  (status-require
    (sp-cheap-filter-state-new test-noise-duration sp-cheap-filter-passes-limit &state))
  (sp-cheap-filter-lp in test-noise-duration 0.2 1 0 &state out)
  (sp-cheap-filter-lp in test-noise-duration 0.2 sp-cheap-filter-passes-limit 0 &state out)
  (sp-cheap-filter-lp in test-noise-duration 0.2 sp-cheap-filter-passes-limit 0 &state out)
  (sp-cheap-filter-state-free &state)
  (label exit status-return))

(define (test-sp-cheap-noise-event) status-t
  status-declare
  (declare
    out sp-block-t
    cut-mod (array sp-sample-t test-noise-duration)
    amod (array sp-sample-t test-noise-duration)
    i sp-time-t
    config sp-cheap-noise-event-config-t*)
  (sp-declare-event event)
  (free-on-error-init 2)
  (status-require (sp-cheap-noise-event-config-new &config))
  (free-on-error1 config)
  (status-require (sp-block-new 2 test-noise-duration &out))
  (free-on-error &out sp-block-free)
  (for ((set i 0) (< i test-noise-duration) (set+ i 1))
    (set
      (array-get cut-mod i) (if* (< i (/ test-noise-duration 2)) 0.01 0.1)
      (array-get cut-mod i) 0.08
      (array-get amod i) 1.0))
  (struct-set *config
    type sp-state-variable-filter-lp
    amp 1
    amod amod
    cut-mod cut-mod
    q-factor 0.1
    channels 2
    amp 1)
  (struct-set event end test-noise-duration data &config prepare sp-cheap-noise-event-prepare)
  (status-require (event.prepare &event))
  (status-require (event.generate 0 test-noise-duration out &event))
  (sp-block-free &out)
  (label exit (if status-is-failure free-on-error-free) status-return))

(pre-define sp-seq-event-count 2)

(define (test-sp-seq) status-t
  status-declare
  (declare out sp-block-t i sp-time-t)
  (sp-declare-event-list events)
  (status-require (sp-event-list-add &events (test-helper-event 0 40 1)))
  (status-require (sp-event-list-add &events (test-helper-event 41 100 2)))
  (status-require (sp-block-new 2 100 &out))
  (sp-seq-events-prepare &events)
  (sp-seq 0 50 out &events)
  (sp-seq 50 100 (sp-block-with-offset out 50) &events)
  (test-helper-assert "block contents 1 event 1"
    (and (= 1 (array-get out.samples 0 0)) (= 1 (array-get out.samples 0 39))))
  (test-helper-assert "block contents 1 gap" (= 0 (array-get out.samples 0 40)))
  (test-helper-assert "block contents 1 event 2"
    (and (= 2 (array-get out.samples 0 41)) (= 2 (array-get out.samples 0 99))))
  (sp-event-list-free &events)
  (sp-block-free &out)
  (label exit status-return))

(define (test-sp-group) status-t
  status-declare
  (declare block sp-block-t m1 sp-time-t* m2 sp-time-t*)
  (sp-declare-event-2 g g1)
  (sp-declare-event-3 e1 e2 e3)
  (status-require (sp-times-new 100 &m1))
  (status-require (sp-times-new 100 &m2))
  (set g.prepare sp-group-prepare g1.start 10 g1.prepare sp-group-prepare)
  (status-require (sp-block-new 2 100 &block))
  (set e1 (test-helper-event 0 20 1) e2 (test-helper-event 20 40 2) e3 (test-helper-event 50 100 3))
  (status-require (sp-group-add &g1 e1))
  (status-require (sp-group-add &g1 e2))
  (status-require (sp-group-add &g g1))
  (status-require (sp-group-add &g e3))
  (status-require (sp-event-memory-init &g 2))
  (sp-event-memory-add1 &g m1)
  (sp-event-memory-add1 &g m2)
  (status-require (g.prepare &g))
  (status-require (g.generate 0 50 block &g))
  (status-require (g.generate 50 100 (sp-block-with-offset block 50) &g))
  (g.free &g)
  (test-helper-assert "block contents event 1"
    (and (= 1 (array-get block.samples 0 10)) (= 1 (array-get block.samples 0 29))))
  (test-helper-assert "block contents event 2"
    (and (= 2 (array-get block.samples 0 30)) (= 2 (array-get block.samples 0 39))))
  (test-helper-assert "block contents gap"
    (and (> 1 (array-get block.samples 0 40)) (> 1 (array-get block.samples 0 49))))
  (test-helper-assert "block contents event 3"
    (and (= 3 (array-get block.samples 0 50)) (= 3 (array-get block.samples 0 99))))
  (sp-block-free &block)
  (label exit status-return))

(pre-define test-wave-event-duration 100)

(define (test-sp-wave-event) status-t
  "sp wave values were taken from printing index and value of the result array.
   sp-plot-samples can plot the result"
  status-declare
  (declare
    out sp-block-t
    fmod (array sp-time-t test-wave-event-duration)
    amod1 (array sp-sample-t test-wave-event-duration)
    amod2 (array sp-sample-t test-wave-event-duration)
    config sp-wave-event-config-t*)
  (sp-declare-event event)
  (free-on-error-init 2)
  (status-require (sp-wave-event-config-new &config))
  (free-on-error1 config)
  (status-require (sp-block-new 2 test-wave-event-duration &out))
  (free-on-error &out sp-block-free)
  (for-each-index i test-wave-event-duration
    (set (array-get fmod i) 2000 (array-get amod1 i) 1 (array-get amod2 i) 0.5))
  (struct-set *config wvf sp-sine-table wvf-size sp-rate fmod fmod amp 1 amod amod1 channels 2)
  (array-set config:channel-config 1 (sp-channel-config 0 10 10 1 amod2))
  (struct-set event start 0 end test-wave-event-duration data config prepare sp-wave-event-prepare)
  (status-require (event.prepare &event))
  (status-require (event.generate 0 30 out event.data))
  (status-require
    (event.generate 30 test-wave-event-duration (sp-block-with-offset out 30) event.data))
  (sc-comment (sp-plot-samples (array-get out.samples 0) test-wave-event-duration))
  (sp-block-free &out)
  (label exit (if status-is-failure free-on-error-free) status-return))

(define (test-render-block) status-t
  status-declare
  (declare
    out sp-block-t
    frq (array sp-time-t test-wave-event-duration)
    amod (array sp-sample-t test-wave-event-duration)
    i sp-time-t
    rc sp-render-config-t
    config sp-wave-event-config-t*)
  (sp-declare-event event)
  (free-on-error-init 2)
  (set rc (sp-render-config sp-channels sp-rate sp-rate) rc.block-size 40)
  (for ((set i 0) (< i test-wave-event-duration) (set+ i 1))
    (set (array-get frq i) 1500 (array-get amod i) 1))
  (status-require (sp-wave-event-config-new &config))
  (free-on-error1 config)
  (status-require (sp-block-new 1 test-wave-event-duration &out))
  (free-on-error &out sp-block-free)
  (struct-set *config wvf sp-sine-table wvf-size sp-rate fmod frq amp 1 amod amod channels 1)
  (struct-set event start 0 end test-wave-event-duration data config prepare sp-wave-event-prepare)
  (sc-comment (sp-render-file event test-wave-event-duration rc "/tmp/test.wav"))
  (sp-render-block event 0 test-wave-event-duration rc &out)
  (sc-comment (sp-block-plot-1 out))
  (sp-block-free &out)
  (label exit (if status-is-failure free-on-error-free) status-return))

(define (test-path) status-t
  (declare samples sp-sample-t* times sp-time-t*)
  status-declare
  (status-require (sp-path-samples-2 &samples 100 (sp-path-line 10 1) (sp-path-line 100 0)))
  (status-require (sp-path-times-2 &times 100 (sp-path-line 10 1) (sp-path-line 100 0)))
  (label exit status-return))

(pre-define (feq a b) (sp-sample-nearly-equal a b 0.01))
(declare rs sp-random-state-t)

(define (u64-from-array-test size) (uint8-t sp-time-t)
  (declare bits-in uint64-t bits-out uint64-t)
  (set
    bits-in 9838263505978427528u
    bits-out (sp-u64-from-array (convert-type &bits-in uint8-t*) size))
  (return (= 0 (memcmp (convert-type &bits-in uint8-t*) (convert-type &bits-out uint8-t*) size))))

(define (test-times) status-t
  status-declare
  (declare
    size sp-time-t
    a-temp sp-time-t*
    a (array sp-time-t 8 1 2 3 4 5 6 7 8)
    b (array sp-time-t 8 0 0 0 0 0 0 0 0)
    b-size sp-time-t
    bits sp-time-t*
    s sp-random-state-t)
  (set a-temp 0 size 8 s (sp-random-state-new 123))
  (sp-times-multiplications 1 3 size a)
  (test-helper-assert "multiplications" (= 81 (array-get a 4)))
  (sp-times-additions 1 3 size a)
  (test-helper-assert "additions" (= 13 (array-get a 4)))
  (declare indices (array sp-time-t 3 1 2 4))
  (sp-times-select a indices 3 a)
  (test-helper-assert "select"
    (and (= 4 (array-get a 0)) (= 7 (array-get a 1)) (= 13 (array-get a 2))))
  (sp-times-set-1 a size 1039 a)
  (status-require (sp-times-new (* 8 (sizeof sp-time-t)) &bits))
  (sp-times-bits->times a (* 8 (sizeof sp-time-t)) bits)
  (test-helper-assert "bits->times"
    (and (= 1 (array-get bits 0)) (= 1 (array-get bits 3))
      (= 0 (array-get bits 4)) (= 1 (array-get bits 10))))
  (free bits)
  (sp-times-multiplications 1 3 size a)
  (sp-times-shuffle &s a size)
  (set s (sp-random-state-new 12))
  (sp-times-random-binary &s size a)
  (sp-times-multiplications 1 3 size a)
  (set s (sp-random-state-new 113))
  (sp-times-select-random &s a size b &b-size)
  (label exit (free a-temp) status-return))

(pre-define test-stats-a-size 8)

(define (test-statistics) status-t
  status-declare
  (declare
    size sp-time-t
    a (array sp-time-t test-stats-a-size 1 2 3 4 5 6 7 8)
    as (array sp-sample-t test-stats-a-size 1 2 3 4 5 6 7 8)
    repetition-1 (array sp-time-t test-stats-a-size 1 1 1 1 1 1 1 1)
    repetition-2 (array sp-time-t test-stats-a-size 1 2 3 4 5 6 7 8)
    inhar-1 (array sp-time-t test-stats-a-size 2 4 6 8 10 12 14 16)
    inhar-2 (array sp-time-t test-stats-a-size 2 4 6 8 10 12 13 16)
    inhar-3 (array sp-time-t test-stats-a-size 2 3 6 8 10 12 13 16)
    inhar-results (array sp-sample-t 3)
    stat-out sp-sample-t)
  (set size test-stats-a-size)
  (sp-stat-times-mean a size &stat-out)
  (test-helper-assert "mean" (feq 4.5 stat-out))
  (sp-stat-times-deviation a size &stat-out)
  (test-helper-assert "deviation" (feq 2.29 stat-out))
  (sp-stat-times-center a size &stat-out)
  (test-helper-assert "center" (feq 4.54 stat-out))
  (sp-stat-times-median a size &stat-out)
  (test-helper-assert "median" (feq 4.5 stat-out))
  (sp-stat-times-skewness a size &stat-out)
  (test-helper-assert "skewness" (feq 0.0 stat-out))
  (sp-stat-times-kurtosis a size &stat-out)
  (test-helper-assert "kurtosis" (feq 1.76 stat-out))
  (sp-stat-samples-mean as size &stat-out)
  (test-helper-assert "samples mean" (feq 4.5 stat-out))
  (sp-stat-samples-deviation as size &stat-out)
  (test-helper-assert "samples deviation" (feq 2.29 stat-out))
  (sp-stat-samples-center as size &stat-out)
  (test-helper-assert "samples center" (feq 4.66 stat-out))
  (sp-stat-samples-median as size &stat-out)
  (test-helper-assert "samples median" (feq 4.5 stat-out))
  (sp-stat-samples-skewness as size &stat-out)
  (test-helper-assert "samples skewness" (feq 0.0 stat-out))
  (sp-stat-samples-kurtosis as size &stat-out)
  (test-helper-assert "samples kurtosis" (feq 1.76 stat-out))
  (sc-comment "repetition-all")
  (sp-stat-times-repetition-all repetition-1 size &stat-out)
  (test-helper-assert "repetition-1 all" (feq 28.0 stat-out))
  (test-helper-assert "repetition-1 all max" (feq (sp-stat-repetition-all-max size) stat-out))
  (sp-stat-times-repetition repetition-1 size 2 &stat-out)
  (test-helper-assert "repetition-1" (feq 6 stat-out))
  (test-helper-assert "repetition-1 max" (feq (sp-stat-repetition-max size 2) stat-out))
  (sp-stat-times-repetition-all repetition-2 size &stat-out)
  (test-helper-assert "repetition-2 all" (feq 0.0 stat-out))
  (sp-stat-samples-repetition-all as size &stat-out)
  (test-helper-assert "samples repetition-all" (feq 0.0 stat-out))
  (sc-comment "inharmonicity")
  (sp-stat-times-inharmonicity inhar-1 size (+ inhar-results 0))
  (sp-stat-times-inharmonicity inhar-2 size (+ inhar-results 1))
  (sp-stat-times-inharmonicity inhar-3 size (+ inhar-results 2))
  (test-helper-assert "inharmonicity relations"
    (< (array-get inhar-results 0) (array-get inhar-results 1) (array-get inhar-results 2)))
  (test-helper-assert "inharmonicity 1" (feq 0.0 (array-get inhar-results 0)))
  (test-helper-assert "inharmonicity 2" (feq 0.0625 (array-get inhar-results 1)))
  (test-helper-assert "inharmonicity 3" (feq 0.125 (array-get inhar-results 2)))
  (label exit status-return))

(define (test-simple-mappings) status-t
  status-declare
  (declare
    size sp-time-t
    a (array sp-time-t 4 1 1 1 1)
    b (array sp-time-t 4 2 2 2 2)
    as (array sp-sample-t 4 1 1 1 1)
    bs (array sp-sample-t 4 2 2 2 2))
  (set size 4)
  (sc-comment "times")
  (sp-times-set-1 a size 0 a)
  (sp-samples-set-1 as size 0 as)
  (test-helper-assert "times set-1" (sp-times-equal-1 a size 0))
  (test-helper-assert "samples set-1" (sp-samples-equal-1 as size 0))
  (sp-times-add-1 a size 1 a)
  (test-helper-assert "add-1" (sp-times-equal-1 a size 1))
  (sp-times-subtract-1 a size 10 a)
  (test-helper-assert "subtract-1" (sp-times-equal-1 a size 0))
  (sp-times-add-1 a size 4 a)
  (sp-times-multiply-1 a size 2 a)
  (test-helper-assert "multiply-1" (sp-times-equal-1 a size 8))
  (sp-times-divide-1 a size 2 a)
  (test-helper-assert "divide-1" (sp-times-equal-1 a size 4))
  (sp-times-set-1 a size 4 a)
  (sp-times-add a size b a)
  (test-helper-assert "add" (sp-times-equal-1 a size 6))
  (sp-times-set-1 a size 4 a)
  (sp-times-subtract a size b a)
  (test-helper-assert "subtract" (sp-times-equal-1 a size 2))
  (sp-times-set-1 a size 4 a)
  (sp-times-multiply a size b a)
  (test-helper-assert "multiply" (sp-times-equal-1 a size 8))
  (sp-times-set-1 a size 4 a)
  (sp-times-divide a size b a)
  (test-helper-assert "divide" (sp-times-equal-1 a size 2))
  (sp-times-set-1 a size 1 a)
  (label exit status-return))

(define (test-random-discrete) status-t
  status-declare
  (declare
    i sp-time-t
    size sp-time-t
    cudist-size sp-time-t
    prob (array sp-time-t 4 0 3 0 3)
    cudist (array sp-time-t 4)
    a (array sp-time-t 8))
  (set cudist-size 4 size 8)
  (sp-times-cusum prob size cudist)
  (sp-times-random-discrete &rs cudist cudist-size 8 a)
  (for ((set i 1) (< i size) (set i (+ 1 i)))
    (test-helper-assert "random-discrete" (or (= 1 (array-get a i)) (= 3 (array-get a i)))))
  (label exit status-return))

(define (test-compositions) status-t
  (declare out sp-time-t** out-size sp-time-t out-sizes sp-time-t* i sp-time-t b sp-time-t*)
  status-declare
  (status-require (sp-times-compositions 5 &out &out-size &out-sizes))
  (for ((set i 0) (< i out-size) (set+ i 1)) (set b (array-get out i)) (free b))
  (free out)
  (label exit status-return))

(define (test-permutations) status-t
  (declare
    in (array sp-time-t 3 1 2 3)
    size sp-time-t
    out-size sp-time-t
    out sp-time-t**
    i sp-time-t
    b sp-time-t*)
  status-declare
  (set size 3)
  (status-require (sp-times-permutations size in size &out &out-size))
  (for ((set i 0) (< i out-size) (set+ i 1)) (set b (array-get out i)) (free b))
  (free out)
  (label exit status-return))

(define (test-sp-seq-parallel) status-t
  "sum 10 wave events"
  status-declare
  (declare
    i sp-time-t
    step-size sp-time-t
    amod sp-sample-t*
    fmod sp-time-t*
    size sp-time-t
    block sp-block-t
    config sp-wave-event-config-t*)
  (sp-declare-event event)
  (sp-declare-event-list events)
  (set size 10000)
  (free-on-error-init 4)
  (status-require (sp-wave-event-config-new &config))
  (free-on-error1 config)
  (status-require (sp-path-samples-2 &amod size (sp-path-move 0 1.0) (sp-path-constant)))
  (free-on-error1 amod)
  (status-require (sp-path-times-2 &fmod size (sp-path-move 0 250) (sp-path-constant)))
  (free-on-error1 fmod)
  (struct-set *config wvf sp-sine-table wvf-size sp-rate fmod fmod amp 1 amod amod channels 2)
  (for ((set i 0) (< i 10) (set+ i 1))
    (struct-set event start 0 end size data config prepare sp-wave-event-prepare)
    (status-require (sp-event-list-add &events event)))
  (status-require (sp-block-new 2 size &block))
  (sp-seq-events-prepare &events)
  (set step-size (/ size 10))
  (for ((set i 0) (< i size) (set+ i step-size))
    (sp-seq-parallel i (+ i step-size) (sp-block-with-offset block i) &events))
  (test-helper-assert "first 1"
    (and (sp-sample-nearly-equal 0 (array-get block.samples 0 0) 0.001)
      (sp-sample-nearly-equal 0 (array-get block.samples 1 0) 0.001)))
  (test-helper-assert "last 1"
    (and (sp-sample-nearly-equal 8.314696 (array-get block.samples 0 (- step-size 1)) 0.001)
      (sp-sample-nearly-equal 8.314696 (array-get block.samples 1 (- step-size 1)) 0.001)))
  (test-helper-assert "first 2"
    (and (sp-sample-nearly-equal 5.0 (array-get block.samples 0 step-size) 0.001)
      (sp-sample-nearly-equal 5.0 (array-get block.samples 1 step-size) 0.001)))
  (test-helper-assert "last 2"
    (and (sp-sample-nearly-equal -4.422887 (array-get block.samples 0 (- (* 2 step-size) 1)) 0.001)
      (sp-sample-nearly-equal -4.422887 (array-get block.samples 1 (- (* 2 step-size) 1)) 0.001)))
  (sp-event-list-free &events)
  (free amod)
  (free fmod)
  (sp-block-free &block)
  (label exit (if status-is-failure free-on-error-free) status-return))

(define (test-sp-map-event-generate start end in out state)
  (status-t sp-time-t sp-time-t sp-block-t sp-block-t void*)
  status-declare
  (sp-block-copy in out)
  status-return)

(define (test-sp-map-event) status-t
  status-declare
  (declare
    size sp-time-t
    block sp-block-t
    amod sp-sample-t*
    config sp-wave-event-config-t*
    map-event-config sp-map-event-config-t*)
  (sp-declare-event-2 parent child)
  (free-on-error-init 2)
  (status-require (sp-wave-event-config-new &config))
  (free-on-error1 config)
  (status-require (sp-map-event-config-new &map-event-config))
  (free-on-error1 map-event-config)
  (set size (* 10 _rate))
  (status-require (sp-path-samples-2 &amod size (sp-path-move 0 1.0) (sp-path-constant)))
  (struct-set *config wvf sp-sine-table wvf-size sp-rate frq 300 fmod 0 amp 1 amod amod channels 1)
  (struct-set child start 0 end size data config prepare sp-wave-event-prepare)
  (status-require (sp-block-new 1 size &block))
  (struct-set *map-event-config event child map-generate test-sp-map-event-generate isolate #t)
  (struct-set parent
    start child.start
    end child.end
    prepare sp-map-event-prepare
    data map-event-config)
  (status-require (parent.prepare &parent))
  (status-require (parent.generate 0 (/ size 2) block &parent))
  (status-require (parent.generate (/ size 2) size block &parent))
  (parent.free &parent)
  (sp-block-free &block)
  (free amod)
  (label exit (if status-is-failure free-on-error-free) status-return))

(define (main) int
  "\"goto exit\" can skip events"
  status-declare
  (set rs (sp-random-state-new 3))
  (sp-initialize 3 2 _rate)
  (test-helper-test-one test-sp-noise-event)
  (test-helper-test-one test-sp-wave-event)
  (test-helper-test-one test-sp-cheap-noise-event)
  (test-helper-test-one test-sp-map-event)
  (test-helper-test-one test-sp-group)
  (test-helper-test-one test-sp-seq)
  (test-helper-test-one test-render-block)
  (test-helper-test-one test-moving-average)
  (test-helper-test-one test-statistics)
  (test-helper-test-one test-path)
  (test-helper-test-one test-file)
  (test-helper-test-one test-sp-cheap-filter)
  (test-helper-test-one test-sp-random)
  (test-helper-test-one test-sp-triangle-square)
  (test-helper-test-one test-fft)
  (test-helper-test-one test-spectral-inversion-ir)
  (test-helper-test-one test-base)
  (test-helper-test-one test-spectral-reversal-ir)
  (test-helper-test-one test-convolve)
  (test-helper-test-one test-windowed-sinc)
  (test-helper-test-one test-times)
  (test-helper-test-one test-permutations)
  (test-helper-test-one test-compositions)
  (test-helper-test-one test-simple-mappings)
  (test-helper-test-one test-random-discrete)
  (test-helper-test-one test-sp-seq-parallel)
  (label exit (test-helper-display-summary) (return status.id)))