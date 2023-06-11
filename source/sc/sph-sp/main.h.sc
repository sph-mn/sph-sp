(pre-define
  f128 (sc-insert "long double")
  f64 double
  sp-bool-t uint8-t
  spline-path-value-t sp-sample-t
  sp-memory-error (status-set-goto sp-s-group-sp sp-s-id-memory)
  sp-noise sp-samples-random
  sp-random-state-t sph-random-state-t
  sp-sample->time sp-cheap-round-positive
  sp-s-group-libc "libc"
  sp-s-group-sph "sph"
  sp-s-group-sp "sp"
  sp-s-id-eof 6
  sp-s-id-file-eof 5
  sp-s-id-file-not-implemented 4
  sp-s-id-file-read 3
  sp-s-id-file-write 2
  sp-s-id-input-type 7
  sp-s-id-invalid-argument 9
  sp-s-id-memory 8
  sp-s-id-not-implemented 10
  sp-s-id-undefined 1
  sp-status-declare (status-declare-group sp-s-group-sp)
  srq status-require
  sp-max-frq (/ sp-rate 2)
  (sp-declare-block id) (define id sp-block-t (struct-literal 0))
  (sp-time->sample x) (convert-type x sp-sample-t)
  (sp-subtract a b) (- a b)
  (sp-block-declare a) (begin (declare a sp-block-t) (set a.size 0))
  (sp-cheap-round-positive a) (convert-type (+ 0.5 a) sp-time-t)
  (sp-cheap-floor-positive a) (begin "only works for non-negative values" (convert-type a sp-time-t))
  (sp-cheap-ceiling-positive a) (+ (convert-type a sp-time-t) (< (convert-type a sp-time-t) a))
  (sp-inline-max a b) (if* (> a b) a b)
  (sp-inline-min a b) (if* (< a b) a b)
  (sp-inline-limit x min-value max-value) (sp-inline-max min-value (sp-inline-min max-value x))
  (sp-inline-optional a b) (if* a a b)
  (sp-inline-default a b) (if* (not a) (set a b))
  (sp-inline-absolute-difference a b)
  (begin
    "subtract the smaller number from the greater number,
     regardless of if the smallest is the first or the second argument"
    (if* (> a b) (- a b) (- b a)))
  (sp-inline-abs a) (if* (> 0 a) (* -1 a) a)
  (sp-inline-no-underflow-subtract a b)
  (begin "subtract b from a but return 0 for negative results" (if* (> a b) (- a b) 0))
  (sp-inline-no-zero-divide a b)
  (begin "divide a by b (a / b) but return 0 if b is zero" (if* (= 0 b) 0 (/ a b)))
  (sp-status-set _id) (set status.group sp-s-group-sp status.id _id)
  (sp-status-set-goto id) (begin (sp-status-set id) status-goto)
  (sp-malloc-type count type pointer-address)
  (sph-helper-malloc (* count (sizeof type)) pointer-address)
  (sp-calloc-type count type pointer-address)
  (sph-helper-calloc (* count (sizeof type)) pointer-address)
  (sp-realloc-type count type pointer-address)
  (sph-helper-realloc (* count (sizeof type)) pointer-address)
  (sp-malloc-type-srq count type pointer-address)
  (status-require (sp-malloc-type count type pointer-address))
  (sp-hz->samples x) (/ sp-rate x)
  (sp-samples->hz x) (convert-type (/ sp-rate x) sp-time-t)
  (sp-hz->factor x) (/ (convert-type x sp-sample-t) (convert-type sp-rate sp-sample-t))
  (sp-hz->rad a) (* 2 M_PI a)
  (sp-rad->hz a) (* (/ M_PI 2) a)
  (sp-factor->hz x) (convert-type (* x sp-rate) sp-time-t)
  (sp-optional-array-get array fixed index) (if* array (array-get array index) fixed)
  (sp-sample->unit a) (/ (+ 1 a) 2.0)
  (sp-time-random) (sp-time-random-primitive &sp-random-state)
  (sp-times-random size out) (sp-times-random-primitive &sp-random-state size out)
  (sp-samples-random size out) (sp-samples-random-primitive &sp-random-state size out)
  (sp-units-random size out) (sp-units-random-primitive &sp-random-state size out)
  (sp-times-random-bounded range size out)
  (sp-times-random-bounded-primitive &sp-random-state range size out)
  (sp-samples-random-bounded range size out)
  (sp-samples-random-bounded-primitive &sp-random-state range size out)
  (sp-units-random-bounded range size out)
  (sp-units-random-bounded-primitive &sp-random-state range size out)
  (sp-time-random-bounded range) (sp-time-random-bounded-primitive &sp-random-state range)
  (sp-sample-random) (sp-sample-random-primitive &sp-random-state)
  (sp-sample-random-bounded range) (sp-sample-random-bounded-primitive &sp-random-state range)
  (sp-unit-random) (sp-unit-random-primitive &sp-random-state)
  (error-memory-init register-size) (memreg2-init-named error register-size)
  (local-memory-init register-size) (memreg2-init-named exit register-size)
  error-memory-free (memreg2-free-named error)
  local-memory-free (memreg2-free-named exit)
  (error-memory-add2 address handler) (memreg2-add-named error address handler)
  (local-memory-add2 address handler) (memreg2-add-named exit address handler)
  (error-memory-add address) (error-memory-add2 address free)
  (local-memory-add address) (local-memory-add2 address free)
  (sp-time-odd? a) (bit-and a 1)
  (sp-local-alloc-srq allocator size pointer-address)
  (begin (srq (allocator size pointer-address)) (local-memory-add (pointer-get pointer-address)))
  (sp-local-units-srq size pointer-address) (sp-local-alloc-srq sp-units-new size pointer-address)
  (sp-local-times-srq size pointer-address) (sp-local-alloc-srq sp-times-new size pointer-address)
  (sp-local-samples-srq size pointer-address)
  (sp-local-alloc-srq sp-samples-new size pointer-address)
  (sp-rate-duration n d)
  (begin
    "return a sample count relative to the current default sample rate sp_rate.
     (rate / d * n)
     example (rt 1 2) returns the count of samples for half a second of sound"
    (convert-type (* (/ sp-rate d) n) sp-time-t))
  (sp-duration n d)
  (begin
    "like rt but works in places where variables are not allowed (ex: array initializers) and before sp_initialize has been called"
    (convert-type (* (/ _sp-rate d) n) sp-time-t))
  spd sp-duration
  sprd sp-rate-duration)

(declare
  sp-block-t
  (type
    (struct
      (channel-count sp-channel-count-t)
      (size sp-time-t)
      (samples (array sp-sample-t* sp-channel-count-limit))))
  sp-channel-count sp-channel-count-t
  sp-cpu-count uint32-t
  sp-file-t (type (struct (file FILE*) (data-size sp-size-t) (channel-count sp-channel-count-t)))
  sp-random-state sp-random-state-t
  sp-rate sp-time-t
  sp-sine-lfo-factor sp-time-t
  sp-sine-table-lfo sp-sample-t*
  sp-sine-table sp-sample-t*
  (sp-block-copy a b) (void sp-block-t sp-block-t)
  (sp-block->file block path rate) (status-t sp-block-t uint8-t* sp-time-t)
  (sp-block-free a) (void sp-block-t*)
  (sp-block-new channel-count sample-count out-block)
  (status-t sp-channel-count-t sp-time-t sp-block-t*)
  (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  (sp-block-zero a) (void sp-block-t)
  (sp-convolve a a-len b b-len result-carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
  (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-deinitialize) void
  (sp-ffti input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-fft input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-file-close-read file) (void sp-file-t)
  (sp-file-close-write file) (void sp-file-t*)
  (sp-file-open-read path file) (status-t uint8-t* sp-file-t*)
  (sp-file-open-write path channel-count sample-rate file)
  (status-t uint8-t* sp-channel-count-t sp-time-t sp-file-t*)
  (sp-file-read file sample-count samples) (status-t sp-file-t sp-time-t sp-sample-t**)
  (sp-file-write file samples sample-count) (status-t sp-file-t* sp-sample-t** sp-time-t)
  (sp-initialize cpu-count channel-count rate) (status-t uint16-t sp-channel-count-t sp-time-t)
  (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-pan->amp value channel) (sp-sample-t sp-sample-t sp-channel-count-t)
  (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-phase current change cycle) (sp-time-t sp-time-t sp-time-t sp-time-t)
  (sp-phase-float current change cycle) (sp-time-t sp-time-t double sp-time-t)
  (sp-random-state-new seed) (sp-random-state-t sp-time-t)
  (sp-sample-max a b) (sp-sample-t sp-sample-t sp-sample-t)
  (sp-sample-min a b) (sp-sample-t sp-sample-t sp-sample-t)
  (sp-sinc a) (sp-sample-t sp-sample-t)
  (sp-sine-lfo size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-sine-period size out) (void sp-time-t sp-sample-t*)
  (sp-sine size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-status-description a) (uint8-t* status-t)
  (sp-status-name a) (uint8-t* status-t)
  (sp-time-factorial a) (sp-time-t sp-time-t)
  (sp-time-max a b) (sp-time-t sp-time-t sp-time-t)
  (sp-time-min a b) (sp-time-t sp-time-t sp-time-t)
  (sp-wave size wvf wvf-size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t*
    sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-window-blackman a width) (sp-sample-t sp-sample-t sp-time-t))

(sc-comment "extra")

(declare
  (sp-triangle t a b) (sp-sample-t sp-time-t sp-time-t sp-time-t)
  (sp-square t size) (sp-sample-t sp-time-t sp-time-t)
  (sp-pan->amp value channel) (sp-sample-t sp-sample-t sp-channel-count-t)
  (sp-normal-random min max) (sp-time-t sp-time-t sp-time-t)
  (sp-time-harmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  (sp-time-harmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  (sp-time-deharmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  (sp-modulo-match index divisors divisor-count) (size-t size-t size-t* size-t)
  (sp-time-expt base exp) (sp-time-t sp-time-t sp-time-t)
  (sp-permutations-max set-size selection-size) (sp-size-t sp-size-t sp-size-t)
  (sp-compositions-max sum) (sp-size-t sp-size-t)
  (sp-set-sequence-max set-size selection-size) (sp-size-t sp-size-t sp-size-t))