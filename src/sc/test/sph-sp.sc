(pre-include "sph-sp/sph-sp.h" "sph-sp/sph/filesystem.h" "sph-sp/sph/test.h" "math.h")

(pre-define
  sp-test-helper-display-summary
  (if status-is-success (printf "--\ntests finished successfully.\n")
    (printf "\ntests failed. %d %s\n" status.id (sp-status-description status)))
  (feq a b) (sp-sample-nearly-equal a b 0.01)
  _sp-rate 960
  sp-seq-event-count 2
  test-resonator-event-duration 100
  test-stats-a-size 8)

(define error-margin sp-sample-t 0.1 test-file-path char* "/tmp/test-sph-sp-file")

(define (test-helper-event-generate start end out event)
  (status-t sp-time-t sp-time-t void* sp-event-t*)
  status-declare
  (declare
    sample-index sp-time-t
    channel-index sp-channel-count-t
    raw-value uint64-t
    base-value sp-sample-t
    out-block sp-block-t*
    local-index sp-time-t
    sample-value sp-sample-t)
  (set
    out-block (convert-type out sp-block-t*)
    raw-value (convert-type event:config uint64-t)
    base-value (convert-type raw-value sp-sample-t)
    sample-index start)
  (while (< sample-index end)
    (set
      local-index (- sample-index start)
      sample-value (* base-value (convert-type (+ sample-index 1) sp-sample-t))
      channel-index 0)
    (while (< channel-index out-block:channel-count)
      (set+ (array-get out-block:samples channel-index local-index) sample-value)
      (set channel-index (+ channel-index 1)))
    (set sample-index (+ sample-index 1)))
  status-return)

(define (test-helper-event start end number) (sp-event-t sp-time-t sp-time-t sp-time-t)
  (sp-declare-event e)
  (set
    e.start start
    e.end end
    e.generate test-helper-event-generate
    e.config (convert-type (convert-type number uint64-t) void*))
  (return e))

(define (test-base) status-t
  status-declare
  (test-helper-assert "input 0.5" (sp-sample-nearly-equal 0.63662 (sp-sinc 0.5) error-margin))
  (test-helper-assert "input 1" (sp-sample-nearly-equal 1.0 (sp-sinc 0) error-margin))
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

(define (test-convolve-smaller) status-t
  "test sp-convolve with a kernel smaller than input"
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
  (status-require (sph-calloc (* result-len (sizeof sp-sample-t)) &result))
  (memreg-add result)
  (status-require (sph-calloc (* a-len (sizeof sp-sample-t)) &a))
  (memreg-add a)
  (status-require (sph-calloc (* b-len (sizeof sp-sample-t)) &b))
  (memreg-add b)
  (status-require (sph-calloc (* carryover-len (sizeof sp-sample-t)) &carryover))
  (memreg-add carryover)
  (sc-comment "prepare input/output data arrays")
  (array-set* a 2 3 4 5 6)
  (array-set* b 1 2 3)
  (array-set* carryover 0 0 0)
  (sc-comment "test convolve first segment")
  (sp-convolve a a-len b b-len carryover-len carryover result)
  (test-helper-assert "first result"
    (sp-samples-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert "first result carryover"
    (sp-samples-nearly-equal carryover carryover-len expected-carryover carryover-len error-margin))
  (sc-comment "test convolve second segment")
  (array-set* a 8 9 10 11 12)
  (array-set* expected-result 35 43 52 58 64)
  (array-set* expected-carryover 57 36 0)
  (sp-convolve a a-len b b-len carryover-len carryover result)
  (test-helper-assert "second result"
    (sp-samples-nearly-equal result result-len expected-result result-len error-margin))
  (test-helper-assert "second result carryover"
    (sp-samples-nearly-equal carryover carryover-len expected-carryover carryover-len error-margin))
  (label exit memreg-free status-return))

(define (test-convolve-larger) status-t
  "test sp-convolve with a kernel larger than input and process more blocks"
  status-declare
  (declare
    in-a sp-sample-t*
    in-b sp-sample-t*
    out sp-sample-t*
    out-control sp-sample-t*
    carryover sp-sample-t*
    carryover-length sp-time-t
    in-a-length sp-time-t
    in-b-length sp-time-t
    block-size sp-time-t
    block-count sp-time-t)
  (memreg-init 5)
  (set
    in-a 0
    in-b 0
    out 0
    carryover 0
    out-control 0
    block-size 10
    block-count 10
    in-a-length 100
    in-b-length 15
    carryover-length in-b-length)
  (status-require (sp-samples-new in-a-length &in-a))
  (memreg-add in-a)
  (status-require (sp-samples-new in-b-length &in-b))
  (memreg-add in-b)
  (status-require (sp-samples-new in-a-length &out))
  (memreg-add out)
  (status-require (sp-samples-new in-a-length &out-control))
  (memreg-add out-control)
  (status-require (sp-samples-new in-b-length &carryover))
  (memreg-add carryover)
  (sp-for-each-index i in-a-length (set (array-get in-a i) i))
  (sp-for-each-index i in-b-length (set (array-get in-b i) (+ 1 i)))
  (sp-convolve in-a in-a-length in-b in-b-length carryover-length carryover out-control)
  (sp-samples-zero carryover in-b-length)
  (sp-for-each-index i block-count
    (sp-convolve (+ (* i block-size) in-a) block-size
      in-b in-b-length carryover-length carryover (+ (* i block-size) out))
    (set carryover-length in-b-length))
  (test-helper-assert "equal to block processing result"
    (sp-samples-nearly-equal out in-a-length out-control in-a-length 0.01))
  (label exit memreg-free status-return))

(define (test-sinc-make-minimum-phase) status-t
  status-declare
  (declare
    length sp-time-t
    center-index sp-time-t
    index sp-time-t
    cutoff-factor sp-sample-t
    impulse-linear (array sp-sample-t 63)
    impulse-minimum (array sp-sample-t 63)
    real-linear (array sp-sample-t 63)
    imag-linear (array sp-sample-t 63)
    real-minimum (array sp-sample-t 63)
    imag-minimum (array sp-sample-t 63)
    energy-linear sp-sample-t
    energy-minimum sp-sample-t
    cumulative-energy-linear sp-sample-t
    cumulative-energy-minimum sp-sample-t
    energy-index-linear sp-time-t
    energy-index-minimum sp-time-t
    tolerance-energy sp-sample-t
    tolerance-magnitude sp-sample-t
    magnitude-linear sp-sample-t
    magnitude-minimum sp-sample-t
    delta-index sp-sample-t)
  (set
    length 63
    center-index (/ (- length 1) 2)
    cutoff-factor 0.1
    tolerance-energy 1.0e-6
    tolerance-magnitude 1.0e-4
    index 0)
  (while (< index length)
    (set
      delta-index (- (convert-type index sp-sample-t) (convert-type center-index sp-sample-t))
      (array-get impulse-linear index)
      (* 2.0 cutoff-factor (sp-sinc (* 2.0 cutoff-factor delta-index)))
      (array-get impulse-minimum index) (array-get impulse-linear index)
      index (+ index 1)))
  (status-require (sp-sinc-make-minimum-phase impulse-minimum length))
  (set index 0)
  (while (< index length)
    (set
      (array-get real-linear index) (array-get impulse-linear index)
      (array-get imag-linear index) 0.0
      (array-get real-minimum index) (array-get impulse-minimum index)
      (array-get imag-minimum index) 0.0
      index (+ index 1)))
  (status-i-require (sp-fft length real-linear imag-linear))
  (status-i-require (sp-fft length real-minimum imag-minimum))
  (set energy-linear 0.0 energy-minimum 0.0 index 0)
  (while (< index length)
    (set
      energy-linear
      (+ energy-linear (* (array-get impulse-linear index) (array-get impulse-linear index)))
      energy-minimum
      (+ energy-minimum (* (array-get impulse-minimum index) (array-get impulse-minimum index)))
      index (+ index 1)))
  (test-helper-assert "sinc_minphase_energy_preserved"
    (<= (fabs (- energy-linear energy-minimum)) tolerance-energy))
  (set index 0)
  (while (< index length)
    (set
      magnitude-linear
      (sqrt
        (+ (* (array-get real-linear index) (array-get real-linear index))
          (* (array-get imag-linear index) (array-get imag-linear index))))
      magnitude-minimum
      (sqrt
        (+ (* (array-get real-minimum index) (array-get real-minimum index))
          (* (array-get imag-minimum index) (array-get imag-minimum index)))))
    (test-helper-assert "sinc_minphase_magnitude_equal"
      (<= (fabs (- magnitude-linear magnitude-minimum)) tolerance-magnitude))
    (set index (+ index 1)))
  (set
    cumulative-energy-linear 0.0
    cumulative-energy-minimum 0.0
    energy-index-linear (- length 1)
    energy-index-minimum (- length 1)
    index 0)
  (while (< index length)
    (set
      cumulative-energy-linear
      (+ cumulative-energy-linear
        (* (array-get impulse-linear index) (array-get impulse-linear index)))
      cumulative-energy-minimum
      (+ cumulative-energy-minimum
        (* (array-get impulse-minimum index) (array-get impulse-minimum index))))
    (if
      (and (>= cumulative-energy-linear (* 0.5 energy-linear)) (= energy-index-linear (- length 1)))
      (set energy-index-linear index))
    (if
      (and (>= cumulative-energy-minimum (* 0.5 energy-minimum))
        (= energy-index-minimum (- length 1)))
      (set energy-index-minimum index))
    (set index (+ index 1)))
  (test-helper-assert "sinc_minphase_energy_earlier" (<= energy-index-minimum energy-index-linear))
  (label exit status-return))

(define (test-resonator-filter-continuity) (status-t void)
  status-declare
  (declare
    in sp-sample-t*
    out sp-sample-t*
    out-control sp-sample-t*
    state sp-convolution-filter-state-t*
    state-control sp-convolution-filter-state-t*
    size sp-time-t
    block-size sp-time-t
    block-count sp-time-t
    cutoff-low sp-sample-t
    cutoff-high sp-sample-t
    transition sp-sample-t
    arguments-buffer (array uint8-t ((* 3 (sizeof sp-sample-t))))
    arguments-length uint8-t
    sample-index sp-size-t
    block-index sp-size-t
    offset sp-time-t)
  (memreg-init 3)
  (set
    size 100
    block-size 10
    block-count 10
    state 0
    state-control 0
    cutoff-low 0.1
    cutoff-high 0.3
    transition 0.1)
  (status-require (sp-samples-new size &in))
  (memreg-add in)
  (status-require (sp-samples-new size &out))
  (memreg-add out)
  (status-require (sp-samples-new size &out-control))
  (memreg-add out-control)
  (set sample-index 0)
  (while (< sample-index size)
    (set
      (array-get in sample-index) (convert-type sample-index sp-sample-t)
      sample-index (+ sample-index 1)))
  (set
    arguments-length (* 3 (sizeof sp-sample-t))
    (pointer-get (convert-type arguments-buffer sp-sample-t*)) cutoff-low
    (pointer-get (+ (convert-type arguments-buffer sp-sample-t*) 1)) cutoff-high
    (pointer-get (+ (convert-type arguments-buffer sp-sample-t*) 2)) transition)
  (status-require
    (sp-convolution-filter in size
      sp-resonator-ir-f arguments-buffer arguments-length &state-control out-control))
  (set block-index 0)
  (while (< block-index block-count)
    (set offset (* block-index block-size))
    (status-require
      (sp-convolution-filter (+ in offset) block-size
        sp-resonator-ir-f arguments-buffer arguments-length &state (+ out offset)))
    (set block-index (+ block-index 1)))
  (test-helper-assert "resonator continuity"
    (sp-samples-nearly-equal out size out-control size 0.01))
  memreg-free
  (sp-convolution-filter-state-free state)
  (sp-convolution-filter-state-free state-control)
  (label exit status-return))

(define (test-resonator-ir-and-filter) (status-t void)
  status-declare
  (declare
    cutoff-low sp-sample-t
    cutoff-high sp-sample-t
    transition sp-sample-t
    ir sp-sample-t*
    ir-len sp-time-t
    state sp-convolution-filter-state-t*
    source (array sp-sample-t 10)
    result (array sp-sample-t 10)
    arguments-buffer (array uint8-t ((* 3 (sizeof sp-sample-t))))
    arguments-length uint8-t)
  (set
    state 0
    (array-get source 0) 3
    (array-get source 1) 4
    (array-get source 2) 5
    (array-get source 3) 6
    (array-get source 4) 7
    (array-get source 5) 8
    (array-get source 6) 9
    (array-get source 7) 0
    (array-get source 8) 1
    (array-get source 9) 2
    (array-get result 0) 0
    (array-get result 1) 0
    (array-get result 2) 0
    (array-get result 3) 0
    (array-get result 4) 0
    (array-get result 5) 0
    (array-get result 6) 0
    (array-get result 7) 0
    (array-get result 8) 0
    (array-get result 9) 0)
  (sc-comment "direct IR construction: normal parameters")
  (set cutoff-low 0.1 cutoff-high 0.3 transition 0.05)
  (status-require (sp-resonator-ir cutoff-low cutoff-high transition &ir &ir-len))
  (test-helper-assert "resonator ir length positive" (> ir-len 0))
  (free ir)
  (sc-comment "direct IR construction: clamping branches (low < 0, high > 0.5, transition <= 0)")
  (set cutoff-low -0.1 cutoff-high 0.6 transition 0.0)
  (status-require (sp-resonator-ir cutoff-low cutoff-high transition &ir &ir-len))
  (test-helper-assert "resonator ir clamped length positive" (> ir-len 0))
  (free ir)
  (sc-comment "IR through ir_f adapter")
  (set
    cutoff-low 0.05
    cutoff-high 0.2
    transition 0.03
    arguments-length (* 3 (sizeof sp-sample-t))
    (pointer-get (convert-type arguments-buffer sp-sample-t*)) cutoff-low
    (pointer-get (+ (convert-type arguments-buffer sp-sample-t*) 1)) cutoff-high
    (pointer-get (+ (convert-type arguments-buffer sp-sample-t*) 2)) transition)
  (status-require (sp-resonator-ir-f arguments-buffer &ir &ir-len))
  (test-helper-assert "resonator ir_f length positive" (> ir-len 0))
  (free ir)
  (sc-comment "filter through convolution: one call to create state and IR")
  (status-require
    (sp-convolution-filter source 10
      sp-resonator-ir-f arguments-buffer arguments-length &state result))
  (sc-comment "second call with identical arguments: exercises state reuse path")
  (status-require
    (sp-convolution-filter source 10
      sp-resonator-ir-f arguments-buffer arguments-length &state result))
  (if state (sp-convolution-filter-state-free state))
  (label exit status-return))

(define (test-moving-average) status-t
  status-declare
  (declare in sp-sample-t* out sp-sample-t* radius sp-time-t size sp-time-t)
  (memreg-init 2)
  (set size 11 radius 4)
  (status-require (sp-path-samples3 &in (- size 1) 5 0 10 0))
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

(define (test-file) status-t
  status-declare
  (declare
    channel-count sp-channel-count-t
    file sp-file-t
    sample-count sp-time-t
    sample-rate sp-time-t)
  (sp-block-declare block-write)
  (sp-block-declare block-read)
  (if (file-exists test-file-path) (unlink test-file-path))
  (set channel-count 2 sample-rate 8000 sample-count 5)
  (status-require (sp-block-new channel-count sample-count &block-write))
  (for-each-index j sp-channel-count-t
    channel-count
    (for-each-index i sp-time-t
      sample-count (set (array-get (array-get block-write.samples j) i) (- sample-count i))))
  (sc-comment "test write")
  (status-require (sp-file-open-write test-file-path channel-count sample-rate &file))
  (status-require (sp-file-write &file block-write.samples sample-count))
  (sp-file-close-write &file)
  (sc-comment "test read")
  (status-require (sp-block-new channel-count sample-count &block-read))
  (status-require (sp-file-open-read test-file-path &file))
  (status-require (sp-file-read file sample-count block-read.samples))
  (for-each-index j sp-channel-count-t
    channel-count
    (test-helper-assert "sp-file-read new file result"
      (sp-samples-nearly-equal (array-get block-write.samples j) sample-count
        (array-get block-read.samples j) sample-count error-margin)))
  (sp-file-close-read file)
  (label exit (sp-block-free &block-write) (sp-block-free &block-read) status-return))

(define (test-fft) (status-t)
  status-declare
  (declare
    real-values (array sp-sample-t 8)
    imag-values (array sp-sample-t 8)
    real-values-original (array sp-sample-t 8)
    imag-values-original (array sp-sample-t 8)
    impulse-real (array sp-sample-t 8)
    impulse-imag (array sp-sample-t 8)
    length sp-time-t
    index sp-time-t
    tolerance sp-sample-t)
  (set length 8 tolerance 1.0e-9 index 0)
  (while (< index length)
    (set
      (array-get real-values index) (+ (* (convert-type index sp-sample-t) 0.1) 0.05)
      (array-get imag-values index) (* (convert-type (- length index) sp-sample-t) 0.01)
      (array-get real-values-original index) (array-get real-values index)
      (array-get imag-values-original index) (array-get imag-values index)
      index (+ index 1)))
  (status-i-require (sp-fft length real-values imag-values))
  (status-i-require (sp-ffti length real-values imag-values))
  (set index 0)
  (while (< index length)
    (test-helper-assert "fft_roundtrip_real"
      (<=
        (fabs
          (- (/ (array-get real-values index) (convert-type length sp-sample-t))
            (array-get real-values-original index)))
        tolerance))
    (test-helper-assert "fft_roundtrip_imag"
      (<=
        (fabs
          (- (/ (array-get imag-values index) (convert-type length sp-sample-t))
            (array-get imag-values-original index)))
        tolerance))
    (set index (+ index 1)))
  (set index 0)
  (while (< index length)
    (set (array-get impulse-real index) 0.0 (array-get impulse-imag index) 0.0 index (+ index 1)))
  (set (array-get impulse-real 0) 1.0)
  (status-i-require (sp-fft length impulse-real impulse-imag))
  (set index 0)
  (while (< index length)
    (test-helper-assert "fft_impulse_real"
      (<= (fabs (- (array-get impulse-real index) 1.0)) tolerance))
    (test-helper-assert "fft_impulse_imag" (<= (fabs (array-get impulse-imag index)) tolerance))
    (set index (+ index 1)))
  (label exit status-return))

(define (test-sp-plot) status-t
  "better test separately as it opens gnuplot windows"
  status-declare
  (declare a (array sp-sample-t 9 0.1 -0.2 0.1 -0.4 0.3 -0.4 0.2 -0.2 0.1))
  (sp-plot-samples a 9)
  (sp-plot-spectrum a 9)
  status-return)

(define (test-sp-triangle-square) status-t
  status-declare
  (declare out-t sp-sample-t* out-s sp-sample-t* size sp-time-t)
  (set size 96000)
  (status-require (sph-calloc (* size (sizeof sp-sample-t*)) &out-t))
  (status-require (sph-calloc (* size (sizeof sp-sample-t*)) &out-s))
  (sp-for-each-index i size
    (set
      (array-get out-t i) (sp-triangle i (/ size 2) (/ size 2))
      (array-get out-s i) (sp-square i size)))
  (test-helper-assert "triangle 0" (= 0 (array-get out-t 0)))
  (test-helper-assert "triangle 1/2" (= 1 (array-get out-t 48000)))
  (test-helper-assert "triangle 1" (sp-sample-nearly-equal 0 (array-get out-t 95999) error-margin))
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
  (sp-samples-random-primitive &s 10 out)
  (sp-samples-random-primitive &s 10 (+ 10 out))
  (test-helper-assert "last value"
    (sp-sample-nearly-equal -0.553401 (array-get out 19) error-margin))
  (label exit status-return))

(define (test-sp-seq) status-t
  status-declare
  (declare out sp-block-t shifted sp-block-t)
  (sp-declare-event-list events)
  (status-require (sp-event-list-add &events (test-helper-event 0 40 1)))
  (status-require (sp-event-list-add &events (test-helper-event 41 100 2)))
  (status-require (sp-block-new 2 100 &out))
  (sp-seq-events-prepare &events)
  (sp-seq 0 50 &out &events)
  (set shifted (sp-block-with-offset out 50))
  (sp-seq 50 100 &shifted &events)
  (test-helper-assert "block contents 1 event 1"
    (and (= 1 (array-get out.samples 0 0)) (= 40 (array-get out.samples 0 39))))
  (test-helper-assert "block contents 1 gap" (= 0 (array-get out.samples 0 40)))
  (test-helper-assert "block contents 1 event 2"
    (and (= 2 (array-get out.samples 0 41)) (= 118 (array-get out.samples 0 99))))
  (sp-event-list-free &events)
  (sp-block-free &out)
  (label exit status-return))

(define (test-sp-group) status-t
  status-declare
  (declare block sp-block-t shifted sp-block-t m1 sp-time-t* m2 sp-time-t*)
  (sp-declare-event g)
  (sp-declare-event g1)
  (sp-declare-event e1)
  (sp-declare-event e2)
  (sp-declare-event e3)
  (status-require (sp-times-new 100 &m1))
  (status-require (sp-times-new 100 &m2))
  (sp-group-event &g)
  (sp-group-event &g1)
  (set g1.start 10)
  (status-require (sp-block-new 2 100 &block))
  (set e1 (test-helper-event 0 20 1) e2 (test-helper-event 20 40 2) e3 (test-helper-event 50 100 3))
  (status-require (sp-group-add &g1 e1))
  (status-require (sp-group-add &g1 e2))
  (status-require (sp-group-add &g g1))
  (status-require (sp-group-add &g e3))
  (status-require (sp-event-memory-ensure &g 2))
  (status-require (sp-event-memory-add &g m1))
  (status-require (sp-event-memory-add &g m2))
  (status-require (g.prepare &g))
  (status-require (g.generate 0 50 &block &g))
  (set shifted (sp-block-with-offset block 50))
  (status-require (g.generate 50 100 &shifted &g))
  (g.free &g)
  (test-helper-assert "block contents event 1"
    (and (= 1 (array-get block.samples 0 10)) (= 20 (array-get block.samples 0 29))))
  (test-helper-assert "block contents event 2"
    (and (= 2 (array-get block.samples 0 30)) (= 20 (array-get block.samples 0 39))))
  (test-helper-assert "block contents gap"
    (and (= 0 (array-get block.samples 0 40)) (= 0 (array-get block.samples 0 49))))
  (test-helper-assert "block contents event 3"
    (and (= 3 (array-get block.samples 0 50)) (= 150 (array-get block.samples 0 99))))
  (sp-block-free &block)
  (label exit status-return))

(define (test-path) status-t
  (declare samples sp-sample-t* times sp-time-t* event sp-event-t)
  status-declare
  (status-require (sp-path-samples3 &samples 100 10 0 1 10))
  (status-require (sp-path-times3 &times 100 10 0 1 10))
  (declare x (array sp-sample-t 2 10 20) y (array sp-sample-t 2 3 4) c (array sp-sample-t 2 0 0.1))
  (status-require (sp-envelope-zero &samples 100 2 x y c))
  (status-require (sp-envelope-zero3 &samples 100 10 1))
  (status-require (sp-envelope-scale &times 100 5 2 x y c))
  (status-require (sp-envelope-scale3 &times 100 5 10 1 2 3))
  (sp-event-reset event)
  (sp-event-path-samples3-srq &event &samples 100 10 0 1 10)
  (label exit status-return))

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
    bits sp-time-t*)
  (set a-temp 0 size 8)
  (sp-times-geometric 1 3 size a)
  (test-helper-assert "multiplications" (= 81 (array-get a 4)))
  (sp-times-additions 1 3 size a)
  (test-helper-assert "additions" (= 13 (array-get a 4)))
  (declare indices (array sp-time-t 3 1 2 4))
  (sp-times-select a indices 3 a)
  (test-helper-assert "select"
    (and (= 4 (array-get a 0)) (= 7 (array-get a 1)) (= 13 (array-get a 2))))
  (sp-times-set a size 1039)
  (status-require (sp-times-new (* 8 (sizeof sp-time-t)) &bits))
  (sp-times-bits->times a (* 8 (sizeof sp-time-t)) bits)
  (test-helper-assert "bits->times"
    (and (= 1 (array-get bits 0)) (= 1 (array-get bits 3))
      (= 0 (array-get bits 4)) (= 1 (array-get bits 10))))
  (free bits)
  (sp-times-geometric 1 3 size a)
  (sp-times-shuffle a size)
  (sp-times-random-binary size a)
  (sp-times-geometric 1 3 size a)
  (sp-times-select-random a size b &b-size)
  (label exit (free a-temp) status-return))

(define (test-statistics) status-t
  status-declare
  (declare
    size sp-time-t
    a (array sp-time-t test-stats-a-size 1 2 3 4 5 6 7 8)
    as (array sp-sample-t test-stats-a-size 1 2 3 4 5 6 7 8)
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

(define (test-render-range-block) (status-t void)
  status-declare
  (declare out sp-block-t render-config sp-render-config-t)
  (sp-declare-event event)
  (error-memory-init 1)
  (set
    render-config (sp-render-config sp-channel-count sp-rate sp-rate 0)
    render-config.block-size 40)
  (status-require (sp-block-new 1 test-resonator-event-duration &out))
  (error-memory-add2 &out sp-block-free)
  (set
    event.start 0
    event.end test-resonator-event-duration
    event.config (convert-type 1 void*)
    event.prepare 0
    event.generate test-helper-event-generate
    event.free 0)
  (sp-render-range-block event 0 test-resonator-event-duration render-config &out)
  (test-helper-assert "render range block nonzero"
    (> (sp-samples-max (array-get out.samples 0) test-resonator-event-duration) 0.0))
  (sp-block-free &out)
  (if status-is-failure error-memory-free)
  (label exit status-return))

(define (test-simple-mappings) status-t
  status-declare
  (declare
    size sp-time-t
    a (array sp-time-t 4 1 1 1 1)
    b (array sp-time-t 4 2 2 2 2)
    as (array sp-sample-t 4 1 1 1 1))
  (set size 4)
  (sc-comment "times")
  (sp-times-set a size 0)
  (sp-samples-set as size 0)
  (test-helper-assert "times set" (sp-times-equal a size 0))
  (test-helper-assert "samples set" (sp-samples-equal as size 0))
  (sp-times-add a size 1)
  (test-helper-assert "add" (sp-times-equal a size 1))
  (sp-times-subtract a size 10)
  (test-helper-assert "subtract" (sp-times-equal a size 0))
  (sp-times-add a size 4)
  (sp-times-multiply a size 2)
  (test-helper-assert "multiply" (sp-times-equal a size 8))
  (sp-times-divide a size 2)
  (test-helper-assert "divide" (sp-times-equal a size 4))
  (sp-times-set a size 4)
  (sp-times-add-times a size b)
  (test-helper-assert "add-times" (sp-times-equal a size 6))
  (sp-times-set a size 4)
  (sp-times-subtract-times a size b)
  (test-helper-assert "subtract" (sp-times-equal a size 2))
  (sp-times-set a size 4)
  (sp-times-multiply-times a size b)
  (test-helper-assert "multiply" (sp-times-equal a size 8))
  (sp-times-set a size 4)
  (sp-times-divide-times a size b)
  (test-helper-assert "divide" (sp-times-equal a size 2))
  (sp-times-set a size 1)
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
  (sp-times-random-discrete cudist cudist-size 8 a)
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
  status-declare
  (declare in (array sp-time-t 3 1 2 3) out-size sp-size-t out sp-time-t**)
  (define size sp-size-t 3)
  (status-require (sp-times-permutations size in size &out &out-size))
  (sp-for-each-index i out-size (free (array-get out i)))
  (free out)
  (label exit status-return))

(define (test-sp-resonator-event) (status-t void)
  status-declare
  (declare
    out sp-block-t
    amod1 (array sp-sample-t test-resonator-event-duration)
    amod2 (array sp-sample-t test-resonator-event-duration)
    config sp-resonator-event-config-t*
    sample-index sp-size-t
    channel-config sp-resonator-event-channel-config-t*)
  (sp-declare-event event)
  (error-memory-init 2)
  (status-require (sp-resonator-event-config-new &config))
  (error-memory-add config)
  (status-require (sp-block-new 2 test-resonator-event-duration &out))
  (error-memory-add2 &out sp-block-free)
  (set sample-index 0)
  (while (< sample-index test-resonator-event-duration)
    (set (array-get amod1 sample-index) 0.9 (array-get amod2 sample-index) 0.8)
    (set+ sample-index 1))
  (set
    config:channel-count 2
    config:bandwidth-threshold 50.0
    channel-config (+ config:channel-config 0)
    channel-config:use 1
    channel-config:amp 1.0
    channel-config:amod amod1
    channel-config:frq 2000.0
    channel-config:wdt 10.0
    channel-config (+ config:channel-config 1)
    channel-config:use 1
    channel-config:amp 0.5
    channel-config:amod amod2
    channel-config:frq 2000.0
    channel-config:wdt 200.0
    event.start 0
    event.end test-resonator-event-duration)
  (sp-resonator-event &event config)
  (status-require (event.prepare &event))
  (status-require (event.generate 0 30 &out &event))
  (status-require (event.generate 30 test-resonator-event-duration &out &event))
  (test-helper-assert "resonator amod applied channel 0"
    (feq 0.9 (sp-samples-max (array-get out.samples 0) test-resonator-event-duration)))
  (test-helper-assert "resonator channel 1 in range"
    (>= 1.0 (sp-samples-absolute-max (array-get out.samples 1) test-resonator-event-duration)))
  (test-helper-assert "resonator channel 1 nonzero"
    (> (sp-samples-absolute-max (array-get out.samples 1) test-resonator-event-duration) 0.0))
  (sp-block-free &out)
  (label exit (if status-is-failure error-memory-free) status-return))

(define (test-sp-map-event-generate start end in out state)
  (status-t sp-time-t sp-time-t void* void* void*)
  status-declare
  (sp-block-copy (pointer-get (convert-type in sp-block-t*))
    (pointer-get (convert-type out sp-block-t*)))
  status-return)

(define (test-sp-map-event) (status-t void)
  status-declare
  (declare size sp-time-t block sp-block-t map-event-config sp-map-event-config-t*)
  (sp-declare-event parent)
  (sp-declare-event child)
  (error-memory-init 1)
  (status-require (sp-map-event-config-new &map-event-config))
  (error-memory-add map-event-config)
  (set size (* 10 _sp-rate) child (test-helper-event 0 size 3))
  (status-require (sp-block-new 1 size &block))
  (set
    map-event-config:event child
    map-event-config:map-generate test-sp-map-event-generate
    map-event-config:isolate 1
    parent.start child.start
    parent.end child.end)
  (sp-map-event &parent map-event-config)
  (status-require (parent.prepare &parent))
  (status-require (parent.generate 0 (/ size 2) &block &parent))
  (status-require (parent.generate (/ size 2) size &block &parent))
  (parent.free &parent)
  (sp-block-free &block)
  (if status-is-failure error-memory-free)
  (label exit status-return))

(define (test-sp-seq-parallel-block) (status-t void)
  status-declare
  (declare
    size sp-time-t
    step-size sp-time-t
    event-count sp-time-t
    event-index sp-time-t
    position sp-time-t
    block sp-block-t
    shifted sp-block-t
    event-sum sp-sample-t
    sample-index sp-time-t
    expected-value sp-sample-t
    sample-value-ch0 sp-sample-t
    sample-value-ch1 sp-sample-t)
  (sp-declare-event event)
  (sp-declare-event-list events)
  (set size 1000 event-count 10 step-size (/ size 10))
  (status-require (sp-block-new 2 size &block))
  (set
    event.start 0
    event.end size
    event.prepare 0
    event.generate test-helper-event-generate
    event.free 0
    event-index 0)
  (while (< event-index event-count)
    (set event.config (convert-type (convert-type (+ event-index 1) uintptr-t) void*))
    (status-require (sp-event-list-add &events event))
    (set+ event-index 1))
  (sp-seq-events-prepare &events)
  (set position 0)
  (while (< position size)
    (set shifted (sp-block-with-offset block position))
    (sp-seq-parallel-block position (+ position step-size) shifted &events)
    (set position (+ position step-size)))
  (set event-sum (convert-type (/ (* event-count (+ event-count 1)) 2) sp-sample-t) sample-index 0)
  (while (< sample-index size)
    (set
      expected-value (* event-sum (convert-type (+ sample-index 1) sp-sample-t))
      sample-value-ch0 (array-get block.samples 0 sample-index)
      sample-value-ch1 (array-get block.samples 1 sample-index))
    (test-helper-assert "seq_parallel_block ch0"
      (sp-sample-nearly-equal expected-value sample-value-ch0 0.001))
    (test-helper-assert "seq_parallel_block ch1"
      (sp-sample-nearly-equal expected-value sample-value-ch1 0.001))
    (set sample-index (+ sample-index 1)))
  (sp-event-list-free &events)
  (sp-block-free &block)
  (label exit status-return))

(define (test-sp-pan->amp) status-t
  status-declare
  (test-helper-assert "value 0, channel 0"
    (sp-sample-nearly-equal 1 (sp-pan->amp 0 0) error-margin))
  (test-helper-assert "value 0, channel 1"
    (sp-sample-nearly-equal 0 (sp-pan->amp 0 1) error-margin))
  (test-helper-assert "value 1, channel 0"
    (sp-sample-nearly-equal 0 (sp-pan->amp 1 0) error-margin))
  (test-helper-assert "value 1, channel 1"
    (sp-sample-nearly-equal 1 (sp-pan->amp 1 1) error-margin))
  (test-helper-assert "value 0.5, channel 0"
    (sp-sample-nearly-equal 1 (sp-pan->amp 0.5 0) error-margin))
  (test-helper-assert "value 0.5, channel 0"
    (sp-sample-nearly-equal 1 (sp-pan->amp 0.5 1) error-margin))
  (test-helper-assert "value 0.25, channel 0"
    (sp-sample-nearly-equal 1 (sp-pan->amp 0.25 0) error-margin))
  (test-helper-assert "value 0.25, channel 1"
    (sp-sample-nearly-equal 0.5 (sp-pan->amp 0.25 1) error-margin))
  (label exit status-return))

(define (test-resonator-continuity-2) (status-t)
  status-declare
  (declare
    in sp-sample-t*
    out sp-sample-t*
    out-control sp-sample-t*
    state sp-convolution-filter-state-t*
    state-control sp-convolution-filter-state-t*
    size sp-time-t
    duration-a sp-time-t
    duration-b sp-time-t
    cutoff-low sp-sample-t
    cutoff-high sp-sample-t
    transition sp-sample-t
    arguments-buffer (array uint8-t ((* 3 (sizeof sp-sample-t))))
    arguments-length uint8-t)
  (set
    size 100
    duration-a 30
    duration-b 70
    state 0
    state-control 0
    cutoff-low 0.1
    cutoff-high 0.3
    transition 0.1)
  (status-require (sp-samples-new size &in))
  (status-require (sp-samples-new size &out))
  (status-require (sp-samples-new size &out-control))
  (for-each-index i sp-size-t size (set (array-get in i) (convert-type i sp-sample-t)))
  (set
    arguments-length (* 3 (sizeof sp-sample-t))
    (array-get (convert-type arguments-buffer sp-sample-t*) 0) cutoff-low
    (array-get (convert-type arguments-buffer sp-sample-t*) 1) cutoff-high
    (array-get (convert-type arguments-buffer sp-sample-t*) 2) transition)
  (status-require
    (sp-convolution-filter in size
      sp-resonator-ir-f arguments-buffer arguments-length &state-control out-control))
  (status-require
    (sp-convolution-filter in duration-a
      sp-resonator-ir-f arguments-buffer arguments-length &state out))
  (status-require
    (sp-convolution-filter (+ in duration-a) duration-b
      sp-resonator-ir-f arguments-buffer arguments-length &state (+ out duration-a)))
  (test-helper-assert "resonator two-segment continuity"
    (sp-samples-nearly-equal out size out-control size 0.01))
  (sp-convolution-filter-state-free state)
  (sp-convolution-filter-state-free state-control)
  (free in)
  (free out)
  (free out-control)
  (label exit status-return))

(define (main) int
  "\"goto exit\" can skip events"
  status-declare
  (sp-initialize 3 2 _sp-rate)
  (test-helper-test-one test-fft)
  (test-helper-test-one test-sinc-make-minimum-phase)
  (test-helper-test-one test-sp-resonator-event)
  (test-helper-test-one test-path)
  (test-helper-test-one test-moving-average)
  (test-helper-test-one test-statistics)
  (test-helper-test-one test-sp-pan->amp)
  (test-helper-test-one test-base)
  (test-helper-test-one test-sp-random)
  (test-helper-test-one test-sp-triangle-square)
  (test-helper-test-one test-spectral-inversion-ir)
  (test-helper-test-one test-spectral-reversal-ir)
  (test-helper-test-one test-compositions)
  (test-helper-test-one test-times)
  (test-helper-test-one test-simple-mappings)
  (test-helper-test-one test-random-discrete)
  (test-helper-test-one test-file)
  (test-helper-test-one test-permutations)
  (test-helper-test-one test-render-range-block)
  (sc-comment "resonator")
  (test-helper-test-one test-convolve-smaller)
  (test-helper-test-one test-convolve-larger)
  (test-helper-test-one test-resonator-ir-and-filter)
  (test-helper-test-one test-resonator-filter-continuity)
  (test-helper-test-one test-resonator-continuity-2)
  (sc-comment "sequencer")
  (test-helper-test-one test-sp-group)
  (test-helper-test-one test-sp-seq)
  (test-helper-test-one test-sp-seq-parallel-block)
  (test-helper-test-one test-sp-map-event)
  (label exit (sp-deinitialize) sp-test-helper-display-summary (return status.id)))
