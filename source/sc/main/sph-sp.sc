(pre-define-if-not-defined __USE_POSIX199309)
(pre-include "byteswap.h" "inttypes.h" "string.h")
(sc-comment "configuration")

(pre-define-if-not-defined
  sp-channel-limit 2
  sp-channels-t uint8-t
  sp-file-format (bit-or SF-FORMAT-WAV SF_FORMAT_FLOAT)
  sp-float-t double
  spline-path-time-t sp-time-t
  spline-path-value-t sp-sample-t
  sp-sample-rate-t uint32-t
  sp-samples-sum f64-sum
  sp-sample-t double
  sp-sf-read sf-readf-double
  sp-sf-write sf-writef-double
  sp-time-t uint32-t
  sp-times-random sph-random-u32-array
  sp-times-random-bounded sph-random-u32-bounded-array
  sp-samples-random sph-random-f64-array
  sp-time-random sph-random-u32
  sp-time-random-bounded sph-random-u32-bounded
  sp-sample-random sph-random-f64)

(pre-include "sph/status.c" "sph/spline-path.h"
  "sph/random.c" "sph/array3.c" "sph/array4.c" "sph/memreg-heap.c" "sph/float.c")

(sc-comment "main")

(pre-define
  boolean uint8-t
  f64 double
  sp-file-bit-input 1
  sp-file-bit-output 2
  sp-file-bit-position 4
  sp-file-mode-read 1
  sp-file-mode-write 2
  sp-file-mode-read-write 3
  sp-s-group-libc "libc"
  sp-s-group-sndfile "sndfile"
  sp-s-group-sp "sp"
  sp-s-group-sph "sph"
  sp-random-state-t sph-random-state-t
  sp-random-state-new sph-random-state-new
  sp-s-id-undefined 1
  sp-s-id-file-channel-mismatch 2
  sp-s-id-file-encoding 3
  sp-s-id-file-header 4
  sp-s-id-file-incompatible 5
  sp-s-id-file-incomplete 6
  sp-s-id-eof 7
  sp-s-id-input-type 8
  sp-s-id-memory 9
  sp-s-id-invalid-argument 10
  sp-s-id-not-implemented 11
  sp-s-id-file-closed 11
  sp-s-id-file-position 12
  sp-s-id-file-type 13
  (sp-file-declare a) (begin (declare a sp-file-t) (set a.flags 0))
  (sp-block-declare a) (begin (declare a sp-block-t) (set a.size 0))
  (sp-cheap-round-positive a) (convert-type (+ 0.5 a) sp-time-t)
  (sp-cheap-floor-positive a) (convert-type a sp-time-t)
  (sp-cheap-ceiling-positive a) (+ (convert-type a sp-time-t) (< (convert-type a sp-time-t) a))
  (sp-sine-state-1 size frq amp phs) (sp-wave-state-1 sp-sine-table sp-rate size frq amp phs)
  (sp-sine-state-2 size frq amp1 amp2 phs1 phs2)
  (sp-wave-state-2 sp-sine-table sp-rate size frq amp1 amp2 phs1 phs2)
  (sp-max a b) (if* (> a b) a b)
  (sp-min a b) (if* (< a b) a b))

(declare
  sp-block-t
  (type
    (struct
      (channels sp-channels-t)
      (size sp-time-t)
      (samples (array sp-sample-t* sp-channel-limit))))
  sp-file-t
  (type
    (struct
      (flags uint8-t)
      (sample-rate sp-sample-rate-t)
      (channel-count sp-channels-t)
      (data void*)))
  sp-cpu-count uint32-t
  sp-default-random-state sp-random-state-t
  sp-rate sp-time-t
  sp-channels sp-channels-t
  sp-sine-table sp-sample-t*
  (sp-block-zero a) (void sp-block-t)
  (sp-file-read file sample-count result-block result-sample-count)
  (status-t sp-file-t* sp-time-t sp-sample-t** sp-time-t*)
  (sp-file-write file block sample-count result-sample-count)
  (status-t sp-file-t* sp-sample-t** sp-time-t sp-time-t*)
  (sp-file-position file result-position) (status-t sp-file-t* sp-time-t*)
  (sp-file-position-set file sample-offset) (status-t sp-file-t* sp-time-t)
  (sp-file-open path mode channel-count sample-rate result-file)
  (status-t uint8-t* int sp-channels-t sp-sample-rate-t sp-file-t*)
  (sp-file-close a) (status-t sp-file-t)
  (sp-block->file block path rate) (status-t sp-block-t uint8-t* sp-time-t)
  (sp-block-new channel-count sample-count out-block) (status-t sp-channels-t sp-time-t sp-block-t*)
  (sp-status-description a) (uint8-t* status-t)
  (sp-status-name a) (uint8-t* status-t)
  (sp-sin-lq a) (sp-sample-t sp-float-t)
  (sp-sinc a) (sp-float-t sp-float-t)
  (sp-window-blackman a width) (sp-float-t sp-float-t sp-time-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-fft input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-ffti input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-convolve a a-len b b-len result-carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
  (sp-block-free a) (void sp-block-t)
  (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-initialise cpu-count channels rate) (status-t uint16-t sp-channels-t sp-time-t)
  sp-wave-state-t
  (type
    (struct
      (amp (array sp-sample-t* sp-channel-limit))
      (phs (array sp-time-t sp-channel-limit))
      (frq sp-time-t*)
      (wvf-size sp-time-t)
      (wvf sp-sample-t*)
      (size sp-time-t)
      (channels sp-channels-t)))
  (sp-sine-period size out) (void sp-time-t sp-sample-t*)
  (sp-phase current change cycle) (sp-time-t sp-time-t sp-time-t sp-time-t)
  (sp-phase-float current change cycle) (sp-time-t sp-time-t double sp-time-t)
  (sp-square t size) (sp-sample-t sp-time-t sp-time-t)
  (sp-triangle t a b) (sp-sample-t sp-time-t sp-time-t sp-time-t)
  (sp-wave start duration state out) (void sp-time-t sp-time-t sp-wave-state-t* sp-block-t)
  (sp-wave-state-1 wvf wvf-size size frq amp phs)
  (sp-wave-state-t sp-sample-t* sp-time-t sp-time-t sp-time-t* sp-sample-t* sp-time-t)
  (sp-wave-state-2 wvf wvf-size size frq amp1 amp2 phs1 phs2)
  (sp-wave-state-t sp-sample-t* sp-time-t
    sp-time-t sp-time-t* sp-sample-t* sp-sample-t* sp-time-t sp-time-t)
  (sp-time-expt base exp) (sp-time-t sp-time-t sp-time-t)
  (sp-time-factorial a) (sp-time-t sp-time-t)
  (sp-sequence-max size min-size) (sp-time-t sp-time-t sp-time-t)
  (sp-set-sequence-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (sp-permutations-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (sp-compositions-max sum) (sp-time-t sp-time-t))

(sc-comment "arrays")

(pre-define
  (sp-samples-zero a size) (memset a 0 (* size (sizeof sp-sample-t)))
  (sp-times-zero a size) (memset a 0 (* size (sizeof sp-time-t)))
  (sp-time-interpolate-linear a b t)
  (sp-cheap-round-positive
    (+ (* (- 1 (convert-type t sp-sample-t)) (convert-type a sp-sample-t))
      (* t (convert-type b sp-sample-t))))
  (sp-sample-interpolate-linear a b t) (+ (* (- 1 t) a) (* t b)))

(declare
  (sp-samples-absolute-max in in-size) (sp-sample-t sp-sample-t* sp-time-t)
  (sp-samples-add-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-add a size b out) (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t*)
  (sp-samples-and a b size limit out)
  (void sp-sample-t* sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-display a size) (void sp-sample-t* sp-time-t)
  (sp-samples-divide-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-divide a size b out) (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t*)
  (sp-samples-equal-1 a size n) (uint8-t sp-sample-t* sp-time-t sp-sample-t)
  (sp-samples-every-equal a size n) (uint8-t sp-sample-t* sp-time-t sp-sample-t)
  (sp-samples-multiply-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-multiply a size b out) (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t*)
  (sp-samples-new size out) (status-t sp-time-t sp-sample-t**)
  (sp-samples-or a b size limit out)
  (void sp-sample-t* sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-sort-less a b c) (uint8-t void* ssize-t ssize-t)
  (sp-samples-sort-swap a b c) (void void* ssize-t ssize-t)
  (sp-samples-reverse a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-set-unity-gain in in-size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-square a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-subtract-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-subtract a size b out) (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t*)
  (sp-samples->times in in-size out) (void sp-sample-t* sp-time-t sp-time-t*)
  (sp-samples-xor a b size limit out)
  (void sp-sample-t* sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-copy a size out) (status-t sp-sample-t* sp-time-t sp-sample-t**)
  (sp-samples-differences a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-additions start summand count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-samples-divisions start n count out) (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-samples-scale-y a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-scale-y-sum a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-times-add-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-add a size b out) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-and a b size limit out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-display a size) (void sp-time-t* sp-time-t)
  (sp-times-divide-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-divide a size b out) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-equal-1 a size n) (uint8-t sp-time-t* sp-time-t sp-time-t)
  (sp-times-multiply-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-multiply a size b out) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-new size out) (status-t sp-time-t sp-time-t**)
  (sp-times-or a b size limit out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-set-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-samples-set-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-times-sort-less a b c) (uint8-t void* ssize-t ssize-t)
  (sp-times-sort-swap a b c) (void void* ssize-t ssize-t)
  (sp-times-reverse a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-square a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-subtract-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-subtract a size b out) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-xor a b size limit out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-copy a size out) (status-t sp-time-t sp-time-t sp-time-t**)
  (sp-times-differences a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-cusum a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-time-random-custom state cudist cudist-size range)
  (sp-time-t sp-random-state-t* sp-time-t* sp-time-t sp-time-t)
  (sp-time-random-discrete state cudist cudist-size)
  (sp-time-t sp-random-state-t* sp-time-t* sp-time-t)
  (sp-times-random-discrete state cudist cudist-size count out)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-sample-random-custom state cudist cudist-size range)
  (sp-sample-t sp-random-state-t* sp-time-t* sp-time-t sp-sample-t)
  (sp-times-swap a i1 i2) (void sp-time-t* ssize-t ssize-t)
  (sp-times-sequence-increment-le a size set-size) (void sp-time-t* sp-time-t sp-time-t)
  (sp-times-compositions sum out out-size out-sizes)
  (status-t sp-time-t sp-time-t*** sp-time-t* sp-time-t**)
  (sp-times-permutations size set set-size out out-size)
  (status-t sp-time-t sp-time-t* sp-time-t sp-time-t*** sp-time-t*)
  (sp-times-multiplications start factor count out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-additions start summand count out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-extract-at-indices a indices size out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t*)
  (sp-times-bits->times a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-shuffle state a size) (void sp-random-state-t* sp-time-t* sp-time-t)
  (sp-times-random-binary state size out) (status-t sp-random-state-t* sp-time-t sp-time-t*)
  (sp-times-gt-indices a size n out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t* sp-time-t*)
  (sp-times-extract-random state a size out out-size)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-constant a size value out) (status-t sp-time-t sp-time-t sp-time-t sp-time-t**)
  (sp-u64-from-array a size) (uint64-t uint8-t* sp-time-t)
  (sp-shuffle state swap a size)
  (void sp-random-state-t* (function-pointer void void* size-t size-t) void* size-t)
  (sp-times-scale a a-size factor out) (status-t sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-shuffle-swap a i1 i2) (void void* size-t size-t)
  (sp-samples-smooth a size radius out) (status-t sp-sample-t* sp-time-t sp-time-t sp-sample-t*)
  (sp-times-array-free a size) (void sp-time-t** sp-time-t)
  (sp-samples-array-free a size) (void sp-sample-t** sp-time-t)
  (sp-times-contains a size b) (uint8-t sp-time-t* sp-time-t sp-time-t)
  (sp-times-random-discrete-unique state cudist cudist-size size out)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-sequences base digits size out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-range start end out) (void sp-time-t sp-time-t sp-time-t*)
  (sp-time-round-to-multiple a base) (sp-time-t sp-time-t sp-time-t)
  (sp-times-limit a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*))

(sc-comment "statistics")
(pre-define sp-stat-types-count (+ 1 (- sp-stat-skewness sp-stat-center)))

(declare
  sp-stat-type-t
  (type
    (enum
      ( (sp-stat-center 0u) sp-stat-complexity sp-stat-complexity-width
        sp-stat-deviation sp-stat-inharmonicity sp-stat-kurtosis
        sp-stat-mean sp-stat-median sp-stat-range
        sp-stat-range-min sp-stat-range-max sp-stat-skewness)))
  sp-stat-times-f-t (type (function-pointer uint8-t sp-time-t* sp-time-t sp-sample-t*))
  sp-stat-samples-f-t (type (function-pointer uint8-t sp-sample-t* sp-time-t sp-sample-t*))
  sp-stat2-times-f-t (type (function-pointer uint8-t sp-time-t* sp-time-t* sp-time-t sp-sample-t*))
  sp-stat2-samples-f-t
  (type (function-pointer uint8-t sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*))
  (sp-stat-times a a-size stats size out)
  (status-t sp-time-t* sp-time-t sp-stat-type-t* sp-time-t sp-sample-t*)
  (sp-stat-samples a a-size stats size out)
  (status-t sp-sample-t* sp-time-t sp-stat-type-t* sp-time-t sp-sample-t*)
  (sp-stat-times-range a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-complexity a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-mean a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-deviation a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-median a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-center a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-inharmonicity a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-kurtosis a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-skewness a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-complexity a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-mean a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-deviation a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-inharmonicity a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-median a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-center a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-range a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-kurtosis a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-skewness a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  sp-stat-times-f-array
  (array sp-stat-times-f-t sp-stat-types-count
    sp-stat-times-center sp-stat-times-complexity sp-stat-times-complexity
    sp-stat-times-deviation sp-stat-times-inharmonicity sp-stat-times-kurtosis
    sp-stat-times-mean sp-stat-times-median sp-stat-times-range
    sp-stat-times-range sp-stat-times-range sp-stat-times-skewness)
  sp-stat-samples-f-array
  (array sp-stat-samples-f-t sp-stat-types-count
    sp-stat-samples-center sp-stat-samples-complexity sp-stat-samples-complexity
    sp-stat-samples-deviation sp-stat-samples-inharmonicity sp-stat-samples-kurtosis
    sp-stat-samples-mean sp-stat-samples-median sp-stat-samples-range
    sp-stat-samples-range sp-stat-samples-range sp-stat-samples-skewness)
  (sp-samples-scale->times a size max out) (void sp-sample-t* sp-time-t sp-time-t sp-time-t*))

(sc-comment "filter")

(pre-define
  sp-filter-state-t sp-convolution-filter-state-t
  sp-filter-state-free sp-convolution-filter-state-free
  sp-cheap-filter-passes-limit 8
  (sp-cheap-filter-lp ...) (sp-cheap-filter sp-state-variable-filter-lp __VA_ARGS__)
  (sp-cheap-filter-hp ...) (sp-cheap-filter sp-state-variable-filter-hp __VA_ARGS__)
  (sp-cheap-filter-bp ...) (sp-cheap-filter sp-state-variable-filter-bp __VA_ARGS__)
  (sp-cheap-filter-br ...) (sp-cheap-filter sp-state-variable-filter-br __VA_ARGS__))

(declare
  sp-convolution-filter-ir-f-t (type (function-pointer status-t void* sp-sample-t** sp-time-t*))
  sp-convolution-filter-state-t
  (type
    (struct
      (carryover sp-sample-t*)
      (carryover-len sp-time-t)
      (carryover-alloc-len sp-time-t)
      (ir sp-sample-t*)
      (ir-f sp-convolution-filter-ir-f-t)
      (ir-f-arguments void*)
      (ir-f-arguments-len uint8-t)
      (ir-len sp-time-t)))
  sp-cheap-filter-state-t
  (type
    (struct
      (in-temp sp-sample-t*)
      (out-temp sp-sample-t*)
      (svf-state (array sp-sample-t ((* 2 sp-cheap-filter-passes-limit))))))
  sp-state-variable-filter-t
  (type
    (function-pointer void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*))
  (sp-moving-average in in-size prev next radius out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir-length transition) (sp-time-t sp-float-t)
  (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-float-t sp-float-t sp-time-t* sp-sample-t**)
  (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-bp-br in in-len
    cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  (sp-state-variable-filter-lp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-hp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-bp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-br out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-peak out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-all out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-cheap-filter type in in-size cutoff passes q-factor unity-gain state out)
  (void sp-state-variable-filter-t sp-sample-t*
    sp-time-t sp-float-t sp-time-t sp-float-t uint8-t sp-cheap-filter-state-t* sp-sample-t*)
  (sp-cheap-filter-state-free a) (void sp-cheap-filter-state-t*)
  (sp-cheap-filter-state-new max-size max-passes out-state)
  (status-t sp-time-t sp-time-t sp-cheap-filter-state-t*)
  (sp-filter in in-size cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-filter-state-t** sp-sample-t*))

(sc-comment "plot")
(pre-define (sp-block-plot-1 a) (sp-plot-samples (array-get a.samples 0) a.size))

(declare
  (sp-plot-samples a a-size) (void sp-sample-t* sp-time-t)
  (sp-plot-times a a-size) (void sp-time-t* sp-time-t)
  (sp-plot-samples->file a a-size path) (void sp-sample-t* sp-time-t uint8-t*)
  (sp-plot-times->file a a-size path) (void sp-time-t* sp-time-t uint8-t*)
  (sp-plot-samples-file path use-steps) (void uint8-t* uint8-t)
  (sp-plot-spectrum->file a a-size path) (void sp-sample-t* sp-time-t uint8-t*)
  (sp-plot-spectrum-file path) (void uint8-t*)
  (sp-plot-spectrum a a-size) (void sp-sample-t* sp-time-t))

(sc-comment "sequencer")

(pre-define
  (sp-event-duration a) (- a.end a.start)
  (sp-event-duration-set a duration) (set a.end (+ a.start duration))
  (sp-event-move a start) (set a.end (+ start (- a.end a.start)) a.start start)
  (sp-cheap-noise-event-lp start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-lp __VA_ARGS__)
  (sp-cheap-noise-event-hp start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-hp __VA_ARGS__)
  (sp-cheap-noise-event-bp start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-bp __VA_ARGS__)
  (sp-cheap-noise-event-br start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-br __VA_ARGS__)
  sp-group-size-t uint32-t
  (sp-group-events a) (: (convert-type a.state sp-group-event-state-t*) events)
  (sp-group-memory a) (: (convert-type a.state sp-group-event-state-t*) memory)
  (sp-group-memory-add a pointer)
  (if (array4-not-full (sp-group-memory a)) (array4-add (sp-group-memory a) pointer))
  (sp-group-add a event)
  (if (array4-not-full (sp-group-events a))
    (begin (array4-add (sp-group-events a) event) (if (< a.end event.end) (set a.end event.end))))
  (sp-group-prepare a)
  (sp-seq-events-prepare (struct-get (sp-group-events a) data) (array4-size (sp-group-events a)))
  (sp-event-set-null a) (set a.state 0 a.end 0)
  (sp-group-free a) (if a.state (a.free &a))
  (sp-group-memory-add-2 g a1 a2) (begin (sp-group-memory-add g a1) (sp-group-memory-add g a2))
  (sp-group-memory-add-3 g a1 a2 a3)
  (begin (sp-group-memory-add-2 g a1 a2) (sp-group-memory-add g a3)))

(declare
  sp-event-t struct
  sp-event-t
  (type
    (struct
      sp-event-t
      (state void*)
      (start sp-time-t)
      (end sp-time-t)
      (f (function-pointer void sp-time-t sp-time-t sp-block-t (struct sp-event-t*)))
      (free (function-pointer void (struct sp-event-t*)))))
  sp-event-f-t (type (function-pointer void sp-time-t sp-time-t sp-block-t sp-event-t*))
  (sp-seq-events-prepare data size) (void sp-event-t* sp-time-t)
  (sp-seq start end out events size) (void sp-time-t sp-time-t sp-block-t sp-event-t* sp-time-t)
  (sp-seq-parallel start end out events size)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t* sp-time-t)
  (sp-noise-event start end amp cut-l cut-h trn-l trn-h is-reject resolution random-state out-event)
  (status-t sp-time-t sp-time-t
    sp-sample-t** sp-sample-t* sp-sample-t*
    sp-sample-t* sp-sample-t* uint8-t sp-time-t sp-random-state-t sp-event-t*)
  (sp-events-array-free events size) (void sp-event-t* sp-time-t)
  (sp-wave-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-wave-event start end state out) (status-t sp-time-t sp-time-t sp-wave-state-t sp-event-t*)
  (sp-cheap-noise-event start end amp type cut passes q-factor resolution random-state out-event)
  (status-t sp-time-t sp-time-t
    sp-sample-t** sp-state-variable-filter-t sp-sample-t*
    sp-time-t sp-sample-t sp-time-t sp-random-state-t sp-event-t*))

(array4-declare-type sp-events sp-event-t)
(declare sp-group-event-state-t (type (struct (events sp-events-t) (memory memreg-register-t))))

(declare
  (sp-group-new start event-size memory-size out)
  (status-t sp-time-t sp-group-size-t sp-group-size-t sp-event-t*)
  (sp-group-append a event) (void sp-event-t* sp-event-t)
  (sp-group-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-parallel-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-free a) (void sp-event-t*))

(sc-comment "path")

(pre-define
  sp-path-t spline-path-t
  sp-path-time-t spline-path-time-t
  sp-path-value-t spline-path-value-t
  sp-path-point-t spline-path-point-t
  sp-path-segment-t spline-path-segment-t
  sp-path-segment-count-t spline-path-segment-count-t
  sp-path-line spline-path-line
  sp-path-move spline-path-move
  sp-path-bezier spline-path-bezier
  sp-path-constant spline-path-constant
  sp-path-path spline-path-path
  sp-path-prepare-segments spline-path-prepare-segments
  sp-path-i-line spline-path-i-line
  sp-path-i-move spline-path-i-move
  sp-path-i-bezier spline-path-i-bezier
  sp-path-i-constant spline-path-i-constant
  sp-path-i-path spline-path-i-path
  sp-path-end spline-path-end
  sp-path-size spline-path-size
  sp-path-free spline-path-free
  sp-path-get spline-path-get
  (sp-path-times-constant out size value)
  (sp-path-times-2 out size (sp-path-move 0 value) (sp-path-constant))
  (sp-path-samples-constant out size value)
  (sp-path-samples-2 out size (sp-path-move 0 value) (sp-path-constant)))

(declare
  (sp-path-samples segments segments-count size out)
  (status-t sp-path-segment-t* sp-path-segment-count-t sp-path-time-t sp-sample-t**)
  (sp-path-samples-1 out size s1) (status-t sp-sample-t** sp-time-t sp-path-segment-t)
  (sp-path-samples-2 out size s1 s2)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (sp-path-samples-3 out size s1 s2 s3)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-samples-4 out size s1 s2 s3 s4)
  (status-t sp-sample-t** sp-time-t
    sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times segments segments-count size out)
  (status-t sp-path-segment-t* sp-path-segment-count-t sp-path-time-t sp-time-t**)
  (sp-path-times-1 out size s1) (status-t sp-time-t** sp-time-t sp-path-segment-t)
  (sp-path-times-2 out size s1 s2)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times-3 out size s1 s2 s3)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times-4 out size s1 s2 s3 s4)
  (status-t sp-time-t** sp-time-t
    sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-derivation path x-changes y-changes index out)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-path-t*)
  (sp-path-samples-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-sample-t** sp-path-time-t*)
  (sp-path-times-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-time-t** sp-path-time-t*)
  (sp-path-multiply path x-factor y-factor) (void sp-path-t sp-sample-t sp-sample-t)
  (sp-path-derivations-normalized base count x-changes y-changes out)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-path-t**)
  (sp-path-samples-derivations-normalized path count x-changes y-changes out out-sizes)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-sample-t*** sp-time-t**))

(sc-comment "main 2")
(pre-define (rt n d) (convert-type (* (/ _rate d) n) sp-time-t))

(declare
  sp-render-config-t (type (struct (channels sp-channels-t) (rate sp-time-t) (block-size sp-time-t)))
  (sp-render-config channels rate block-size) (sp-render-config-t sp-channels-t sp-time-t sp-time-t)
  (sp-render-file event start end config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*)
  (sp-render-block event start end config out)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t sp-block-t*)
  (sp-render-quick a file-or-plot) (status-t sp-event-t uint8-t)
  (sp-sine-cluster prt-count amp frq phs ax ay fx fy out)
  (status-t sp-time-t sp-path-t
    sp-path-t sp-time-t* sp-sample-t** sp-sample-t** sp-sample-t** sp-sample-t** sp-event-t*))