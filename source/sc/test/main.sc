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
  (test-helper-assert
    "window-blackman 0 51" (sp-sample-nearly-equal 0 (sp-window-blackman 0 51) error-margin))
  (test-helper-assert
    "window-blackman 25 51" (sp-sample-nearly-equal 1 (sp-window-blackman 25 51) error-margin))
  (label exit
    (return status)))

(define (test-spectral-inversion-ir) status-t
  status-declare
  (declare
    a-len sp-count-t
    a (array sp-sample-t 5 0.1 -0.2 0.3 -0.2 0.1))
  (set a-len 5)
  (sp-spectral-inversion-ir a a-len)
  (test-helper-assert
    "result check"
    (and
      (sp-sample-nearly-equal -0.1 (array-get a 0) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 1) error-margin)
      (sp-sample-nearly-equal 0.7 (array-get a 2) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 3) error-margin)
      (sp-sample-nearly-equal -0.1 (array-get a 4) error-margin)))
  (label exit
    (return status)))

(define (test-spectral-reversal-ir) status-t
  status-declare
  (declare
    a-len sp-count-t
    a (array sp-sample-t 5 0.1 -0.2 0.3 -0.2 0.1))
  (set a-len 5)
  (sp-spectral-reversal-ir a a-len)
  (test-helper-assert
    "result check"
    (and
      (sp-sample-nearly-equal 0.1 (array-get a 0) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 1) error-margin)
      (sp-sample-nearly-equal 0.3 (array-get a 2) error-margin)
      (sp-sample-nearly-equal 0.2 (array-get a 3) error-margin)
      (sp-sample-nearly-equal 0.1 (array-get a 4) error-margin)))
  (label exit
    (return status)))

(define (test-convolve) status-t
  status-declare
  (declare
    a sp-sample-t*
    a-len sp-count-t
    b sp-sample-t*
    b-len sp-count-t
    carryover sp-sample-t*
    carryover-len sp-count-t
    result sp-sample-t*
    result-len sp-count-t
    sample-count sp-count-t
    expected-result (array sp-sample-t (5) 2 7 16 22 28)
    expected-carryover (array sp-sample-t (3) 27 18 0))
  (memreg-init 4)
  (set
    sample-count 5
    b-len 3
    result-len sample-count
    a-len sample-count
    carryover-len b-len)
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
  (test-helper-assert
    "first result"
    (sp-sample-array-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert
    "first result carryover"
    (sp-sample-array-nearly-equal
      carryover carryover-len expected-carryover carryover-len error-margin))
  (sc-comment "test convolve second segment")
  (array-set a 0 8 1 9 2 10 3 11 4 12)
  (array-set expected-result 0 35 1 43 2 52 3 58 4 64)
  (array-set expected-carryover 0 57 1 36 2 0)
  (sp-convolve a a-len b b-len carryover-len carryover result)
  (test-helper-assert
    "second result"
    (sp-sample-array-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert
    "second result carryover"
    (sp-sample-array-nearly-equal
      carryover carryover-len expected-carryover carryover-len error-margin))
  (label exit
    memreg-free
    (return status)))

(define (test-moving-average) status-t
  status-declare
  (declare
    radius sp-count-t
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
  ;(debug-display-sample-array out 3)
  (test-helper-assert
    "moving-average 1.1" (sp-sample-nearly-equal 6.142857142857143 (array-get out 0) error-margin))
  (test-helper-assert
    "moving-average 1.2" (sp-sample-nearly-equal 6.571428571428571 (array-get out 1) error-margin))
  (test-helper-assert
    "moving-average 1.2" (sp-sample-nearly-equal 7 (array-get out 2) error-margin))
  (sc-comment "second run. result number series will be symmetric")
  (array-set out 0 0 1 0 2 0 3 0 4 0)
  (array-set in 0 2 1 2 2 2 3 2 4 2)
  (status-require (sp-moving-average in in-end in in-end 0 0 0 0 1 out))
  (test-helper-assert
    "moving-average 2.1" (sp-sample-nearly-equal 1.3 (array-get out 0) error-margin))
  (test-helper-assert
    "moving-average 2.2" (sp-sample-nearly-equal 2 (array-get out 1) error-margin))
  (test-helper-assert
    "moving-average 2.3" (sp-sample-nearly-equal 2 (array-get out 2) error-margin))
  (test-helper-assert
    "moving-average 2.4" (sp-sample-nearly-equal 2 (array-get out 3) error-margin))
  (test-helper-assert
    "moving-average 2.5" (sp-sample-nearly-equal 1.3 (array-get out 4) error-margin))
  (label exit
    (return status)))

(define (test-windowed-sinc) status-t
  status-declare
  (declare
    transition sp-float-t
    cutoff sp-float-t
    ir sp-sample-t*
    ir-len sp-count-t
    state sp-convolution-filter-state-t*
    source (array sp-sample-t 10 3 4 5 6 7 8 9 0 1 2)
    result (array sp-sample-t 10 0 0 0 0 0 0 0 0 0 0))
  (set
    state 0
    cutoff 0.1
    transition 0.08)
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
  (label exit
    (return status)))

(define (test-file) status-t
  status-declare
  (declare
    channel sp-count-t
    channel-count sp-channel-count-t
    block sp-block-t
    block-2 sp-block-t
    len sp-count-t
    file sp-file-t
    position sp-count-t
    sample-count sp-count-t
    result-sample-count sp-count-t
    sample-rate sp-sample-rate-t
    unequal int8-t)
  (if (file-exists test-file-path) (unlink test-file-path))
  (set
    channel-count 2
    sample-rate 8000
    sample-count 5
    position 0
    channel channel-count)
  (sp-block-set-null block)
  (sp-block-set-null block-2)
  (status-require (sp-block-new channel-count sample-count &block))
  (status-require (sp-block-new channel-count sample-count &block-2))
  (while channel
    (set
      channel (- channel 1)
      len sample-count)
    (while len
      (set
        len (- len 1)
        (array-get (array-get block.samples channel) len) len)))
  (goto exit)
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
  (set
    len channel-count
    unequal 0)
  (while (and len (not unequal))
    (set
      len (- len 1)
      unequal
      (not
        (sp-sample-array-nearly-equal
          (array-get block.samples len)
          sample-count (array-get block-2.samples len) sample-count error-margin))))
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
  (set
    unequal 0
    len channel-count)
  (while (and len (not unequal))
    (set
      len (- len 1)
      unequal
      (not
        (sp-sample-array-nearly-equal
          (array-get block.samples len)
          sample-count (array-get block-2.samples len) sample-count error-margin))))
  (test-helper-assert "sp-file-read existing result" (not unequal))
  (status-require (sp-file-close &file))
  (printf "  open\n")
  (label exit
    (sp-block-free block)
    (sp-block-free block-2)
    (return status)))

(define (test-fft) status-t
  status-declare
  (declare
    a-real (array sp-sample-t 6 -0.6 0.1 0.4 0.8 0 0)
    a-imag (array sp-sample-t 6 0 0 0 0 0 0)
    a-len sp-count-t)
  (set a-len 6)
  (status-require (sp-fft a-len a-real a-imag))
  (status-require (sp-ffti a-len a-real a-imag))
  (label exit
    (return status)))

(define (test-synth) status-t
  status-declare
  (declare
    state sp-count-t*
    config-len sp-count-t
    out1 sp-block-t
    out2 sp-block-t
    channels sp-channel-count-t
    duration sp-count-t
    prt1 sp-synth-partial-t
    prt2 sp-synth-partial-t
    prt3 sp-synth-partial-t
    config (array sp-synth-partial-t 3)
    wvl (array sp-count-t 4 2 2 2 2)
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
  (label exit
    (return status)))

(pre-define
  sp-seq-duration 20
  sp-seq-half-duration (/ sp-seq-duration 2))

(define (test-sp-seq) status-t
  status-declare
  (declare
    events sp-events-t
    state sp-count-t*
    out sp-block-t
    prt sp-synth-partial-t
    config (array sp-synth-partial-t 1)
    wvl (array sp-count-t sp-seq-duration)
    amp (array sp-sample-t sp-seq-duration)
    i sp-count-t
    e sp-event-t)
  (for ((set i 0) (< i sp-seq-duration) (set i (+ 1 i)))
    (set
      (array-get wvl i) 5
      (array-get amp i) 0.5))
  (set
    prt.modifies 0
    prt.start 0
    prt.end sp-seq-duration
    (array-get prt.amp 0) amp
    (array-get prt.wvl 0) wvl
    (array-get prt.phs 0) 0
    (array-get config 0) prt)
  (status-id-require (i-array-allocate-sp-events-t 2 &events))
  (status-require (sp-synth-event 0 sp-seq-half-duration 1 1 config &e))
  (i-array-add events e)
  (status-require (sp-synth-event sp-seq-half-duration sp-seq-duration 1 1 config &e))
  (i-array-add events e)
  (sp-seq-events-prepare events)
  (status-require (sp-block-new 1 sp-seq-duration &out))
  (sp-seq 0 0 sp-seq-half-duration out events)
  (sp-seq sp-seq-half-duration sp-seq-half-duration sp-seq-half-duration out events)
  (for ((set i 0) (< i sp-seq-duration) (set i (+ 1 i)))
    (printf "%f " (array-get (array-get *out.samples i))))
  (label exit
    (return status)))

(define (test-sp-plot) status-t
  status-declare
  (declare a (array sp-sample-t 4 0.1 0.2 0.3 0.4))
  (sp-plot-samples a)
  (label exit
    (return status)))

(define (main) int
  status-declare
  (test-sp-plot)
  (goto exit)
  (sp-initialise)
  (test-helper-test-one test-sp-seq)
  (test-helper-test-one test-synth)
  (test-helper-test-one test-moving-average)
  (test-helper-test-one test-fft)
  (test-helper-test-one test-spectral-inversion-ir)
  (test-helper-test-one test-base)
  (test-helper-test-one test-spectral-reversal-ir)
  (test-helper-test-one test-convolve)
  (test-helper-test-one test-file)
  (test-helper-test-one test-windowed-sinc)
  (label exit
    (test-helper-display-summary)
    (return status.id)))