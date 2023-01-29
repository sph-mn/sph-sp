(pre-include-guard-begin sph-sp-h)
(pre-define-if-not-defined __USE_POSIX199309)
(pre-define-if-not-defined _GNU_SOURCE)

(pre-include "byteswap.h" "inttypes.h"
  "string.h" "sys/types.h" "sph-sp/status.h"
  "sph-sp/array3.c" "sph-sp/float.h" "sph-sp/hashtable.c"
  "sph-sp/helper.h" "sph-sp/memreg.c" "sph-sp/random.h" "sph-sp/set.c" "sph-sp/spline-path.h")

(sc-comment "configuration")

(pre-define-if-not-defined
  sp-channel-count-t uint8-t
  sp-channel-limit 2
  sp-cheap-filter-passes-limit 8
  spline-path-value-t sp-sample-t
  sp-random-seed 1557083953
  sp-samples-nearly-equal sph-f64-array-nearly-equal
  sp-sample-nearly-equal sph-f64-nearly-equal
  sp-sample-random-primitive sph-random-f64-1to1
  sp-samples-random-bounded-primitive sph-random-f64-bounded-array
  sp-samples-random-primitive sph-random-f64-array-1to1
  sp-samples-sum sph-f64-sum
  sp-sample-t double
  sp-time-half-t uint16-t
  sp-time-random-bounded-primitive sph-random-u32-bounded
  sp-time-random-primitive sph-random-u32
  sp-times-random-bounded-primitive sph-random-u32-bounded-array
  sp-times-random-primitive sph-random-u32-array
  sp-time-t uint32-t
  sp-unit-random-primitive sph-random-f64-0to1
  sp-units-random-primitive sph-random-f64-array-0to1
  sp-unit-t double)

(sc-comment "main")

(pre-define
  (error-memory-init register-size) (memreg2-init-named error register-size)
  (local-memory-init register-size) (memreg2-init-named exit register-size)
  error-memory-free (memreg2-free-named error)
  local-memory-free (memreg2-free-named exit)
  (error-memory-add2 address handler) (memreg2-add-named error address handler)
  (local-memory-add2 address handler) (memreg2-add-named exit address handler)
  (error-memory-add address) (error-memory-add2 address free)
  (local-memory-add address) (local-memory-add2 address free))

(pre-define
  sp-bool-t uint8-t
  f64 double
  f128 (sc-insert "long double")
  sp-s-group-libc "libc"
  sp-s-group-sp "sp"
  sp-s-group-sph "sph"
  sp-s-id-undefined 1
  sp-s-id-file-write 2
  sp-s-id-file-read 3
  sp-s-id-file-not-implemented 4
  sp-s-id-file-eof 5
  sp-s-id-eof 7
  sp-s-id-input-type 8
  sp-s-id-memory 9
  sp-s-id-invalid-argument 10
  sp-s-id-not-implemented 11
  sp-random-state-t sph-random-state-t
  sp-noise sp-samples-random
  (sp-block-declare a) (begin (declare a sp-block-t) (set a.size 0))
  (sp-cheap-round-positive a) (convert-type (+ 0.5 a) sp-time-t)
  (sp-cheap-floor-positive a) (begin "only works for non-negative values" (convert-type a sp-time-t))
  (sp-cheap-ceiling-positive a) (+ (convert-type a sp-time-t) (< (convert-type a sp-time-t) a))
  (sp-max a b) (if* (> a b) a b)
  (sp-min a b) (if* (< a b) a b)
  (sp-limit x min-value max-value) (sp-max min-value (sp-min max-value x))
  (sp-absolute-difference a b)
  (begin
    "subtract the smaller number from the greater number,
     regardless of if the smallest is the first or the second argument"
    (if* (> a b) (- a b) (- b a)))
  (sp-abs a) (if* (> 0 a) (* -1 a) a)
  (sp-no-underflow-subtract a b)
  (begin "subtract b from a but return 0 for negative results" (if* (> a b) (- a b) 0))
  (sp-no-zero-divide a b)
  (begin "divide a by b (a / b) but return 0 if b is zero" (if* (= 0 b) 0 (/ a b)))
  (sp-status-set _id) (set status.group sp-s-group-sp status.id _id)
  (sp-status-set-goto id) (begin (sp-status-set id) status-goto)
  (sp-malloc-type count type pointer-address)
  (sph-helper-malloc (* count (sizeof type)) pointer-address)
  (sp-calloc-type count type pointer-address)
  (sph-helper-calloc (* count (sizeof type)) pointer-address)
  (sp-realloc-type count type pointer-address)
  (sph-helper-realloc (* count (sizeof type)) pointer-address)
  (sp-hz->samples x) (/ sp-rate x)
  (sp-samples->hz x) (convert-type (/ sp-rate x) sp-time-t)
  (sp-hz->factor x) (/ (convert-type x sp-sample-t) (convert-type sp-rate sp-sample-t))
  (sp-factor->hz x) (convert-type (* x sp-rate) sp-time-t)
  (sp-array-or-fixed array fixed index) (if* array (array-get array index) fixed)
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
  (sp-local-alloc allocator size pointer-address)
  (begin (srq (allocator size pointer-address)) (local-memory-add (pointer-get pointer-address)))
  (sp-local-units size pointer-address) (sp-local-alloc sp-units-new size pointer-address)
  (sp-local-times size pointer-address) (sp-local-alloc sp-times-new size pointer-address)
  (sp-local-samples size pointer-address) (sp-local-alloc sp-samples-new size pointer-address)
  (sp-event-alloc event-pointer allocator pointer-address)
  (begin
    (srq (allocator pointer-address))
    (sp-event-memory-add event-pointer (pointer-get pointer-address)))
  (sp-event-alloc1 event-pointer allocator size pointer-address)
  (begin
    (srq (allocator size pointer-address))
    (sp-event-memory-add event-pointer (pointer-get pointer-address)))
  (sp-event-malloc event-pointer size pointer-address)
  (begin
    (srq (sph-helper-malloc size pointer-address))
    (sp-event-memory-add event-pointer (pointer-get pointer-address)))
  (sp-event-malloc-type-n* event-pointer count type pointer-address)
  (sp-event-malloc event-pointer (* count (sizeof type)) pointer-address)
  (sp-event-malloc-type event-pointer type pointer-address)
  (sp-event-malloc event-pointer (sizeof type) pointer-address)
  (sp-event-samples event-pointer size pointer-address)
  (sp-event-alloc event-pointer sp-samples-new size pointer-address)
  (sp-event-times event-pointer size pointer-address)
  (sp-event-alloc event-pointer sp-times-new size pointer-address)
  (sp-event-units event-pointer size pointer-address)
  (sp-event-alloc event-pointer sp-units-new size pointer-address))

(declare
  sp-block-t
  (type
    (struct
      (channel-count sp-channel-count-t)
      (size sp-time-t)
      (samples (array sp-sample-t* sp-channel-limit))))
  sp-file-t (type (struct (file FILE*) (data-size size-t) (channel-count sp-channel-count-t)))
  sp-cpu-count uint32-t
  sp-random-state sp-random-state-t
  sp-rate sp-time-t
  sp-channel-count sp-channel-count-t
  sp-sine-table sp-sample-t*
  sp-sine-table-lfo sp-sample-t*
  sp-sine-lfo-factor sp-time-t
  (sp-wave size wvf wvf-size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t*
    sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-sine size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-random-state-new seed) (sp-random-state-t sp-time-t)
  (sp-block-zero a) (void sp-block-t)
  (sp-block-copy a b) (void sp-block-t sp-block-t)
  (sp-file-open-write path channel-count sample-rate file)
  (status-t uint8-t* sp-channel-count-t sp-time-t sp-file-t*)
  (sp-file-write file samples sample-count) (status-t sp-file-t* sp-sample-t** sp-time-t)
  (sp-file-close-write file) (void sp-file-t*)
  (sp-file-read file sample-count samples) (status-t sp-file-t sp-time-t sp-sample-t**)
  (sp-file-open-read path file) (status-t uint8-t* sp-file-t*)
  (sp-file-close-read file) (void sp-file-t)
  (sp-block->file block path rate) (status-t sp-block-t uint8-t* sp-time-t)
  (sp-block-new channel-count sample-count out-block)
  (status-t sp-channel-count-t sp-time-t sp-block-t*)
  (sp-status-description a) (uint8-t* status-t)
  (sp-status-name a) (uint8-t* status-t)
  (sp-sinc a) (sp-sample-t sp-sample-t)
  (sp-window-blackman a width) (sp-sample-t sp-sample-t sp-time-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-fft input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-ffti input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-convolve a a-len b b-len result-carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
  (sp-block-free a) (void sp-block-t*)
  (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-initialize cpu-count channel-count rate) (status-t uint16-t sp-channel-count-t sp-time-t)
  (sp-sine-period size out) (void sp-time-t sp-sample-t*)
  (sp-phase current change cycle) (sp-time-t sp-time-t sp-time-t sp-time-t)
  (sp-phase-float current change cycle) (sp-time-t sp-time-t double sp-time-t)
  (sp-square t size) (sp-sample-t sp-time-t sp-time-t)
  (sp-triangle t a b) (sp-sample-t sp-time-t sp-time-t sp-time-t)
  (sp-time-expt base exp) (sp-time-t sp-time-t sp-time-t)
  (sp-time-factorial a) (sp-time-t sp-time-t)
  (sp-pan->amp value channel) (sp-sample-t sp-sample-t sp-channel-count-t)
  (sp-normal-random min max) (sp-time-t sp-time-t sp-time-t)
  (sp-time-harmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  (sp-time-deharmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  (sp-sine-lfo size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-modulo-match index divisors divisor-count) (size-t size-t size-t* size-t))

(sc-comment "arrays")

(pre-define
  (sp-samples-zero a size) (memset a 0 (* size (sizeof sp-sample-t)))
  (sp-times-zero a size) (memset a 0 (* size (sizeof sp-time-t)))
  (sp-time-interpolate-linear a b t)
  (sp-cheap-round-positive
    (+ (* (- 1 (convert-type t sp-sample-t)) (convert-type a sp-sample-t))
      (* t (convert-type b sp-sample-t))))
  (sp-sample-interpolate-linear a b t) (+ (* (- 1 t) a) (* t b))
  (sp-sequence-set-equal a b)
  (and (= a.size b.size)
    (or (and (= 0 a.size) (= 0 b.size)) (= 0 (memcmp a.data b.data (* a.size (sizeof sp-time-t)))))))

(sc-comment "sp-sequence-set and sp-time-set can be used for deduplication")

(declare
  sp-sequence-set-key-t (type (struct (size sp-time-t) (data uint8-t*)))
  (sp-u64-from-array a size) (uint64-t uint8-t* sp-time-t))

(define sp-sequence-set-null sp-sequence-set-key-t (struct-literal 0 0))

(define (sp-sequence-set-hash a memory-size) (uint64-t sp-sequence-set-key-t sp-time-t)
  (return (modulo (sp-u64-from-array a.data a.size) memory-size)))

(sph-set-declare-type-nonull sp-sequence-set sp-sequence-set-key-t
  sp-sequence-set-hash sp-sequence-set-equal sp-sequence-set-null 2)

(sph-set-declare-type sp-time-set sp-time-t sph-set-hash-integer sph-set-equal-integer 0 1 2)

(sph-hashtable-declare-type sp-sequence-hashtable sp-sequence-set-key-t
  sp-time-t sp-sequence-set-hash sp-sequence-set-equal 2)

(declare
  (sp-samples-absolute-max in in-size) (sp-sample-t sp-sample-t* sp-time-t)
  (sp-samples-add-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-add a size b out) (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t*)
  (sp-samples-copy a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
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
  (sp-samples-set-gain a a-len amp) (void sp-sample-t* sp-time-t sp-sample-t)
  (sp-samples-set-unity-gain in out size) (void sp-sample-t* sp-sample-t* sp-time-t)
  (sp-samples-square a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-subtract-1 a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-subtract a size b out) (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t*)
  (sp-samples->times in in-size out) (void sp-sample-t* sp-time-t sp-time-t*)
  (sp-samples-xor a b size limit out)
  (void sp-sample-t* sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-samples-duplicate a size out) (status-t sp-sample-t* sp-time-t sp-sample-t**)
  (sp-samples-differences a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-additions start summand count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-samples-divisions start n count out) (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-samples-scale-y a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-times-scale-y a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-scale-sum a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-samples-scale-sum a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  (sp-times-absolute-max in size) (sp-time-t sp-time-t* sp-time-t)
  (sp-times-add-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-add a size b out) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-copy a size out) (void sp-time-t* sp-time-t sp-time-t*)
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
  (sp-times-sum a size) (sp-time-t sp-time-t* sp-time-t)
  (sp-times-reverse a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-square a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-subtract-1 a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-subtract a size b out) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-xor a b size limit out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-duplicate a size out) (status-t sp-time-t sp-time-t sp-time-t**)
  (sp-times-differences a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-cusum a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-time-random-custom cudist cudist-size range) (sp-time-t sp-time-t* sp-time-t sp-time-t)
  (sp-time-random-discrete cudist cudist-size) (sp-time-t sp-time-t* sp-time-t)
  (sp-times-random-discrete cudist cudist-size count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-sample-random-custom cudist cudist-size range) (sp-sample-t sp-time-t* sp-time-t sp-sample-t)
  (sp-times-swap a i1 i2) (void sp-time-t* ssize-t ssize-t)
  (sp-times-sequence-increment a size set-size) (void sp-time-t* sp-time-t sp-time-t)
  (sp-times-compositions sum out out-size out-sizes)
  (status-t sp-time-t sp-time-t*** sp-time-t* sp-time-t**)
  (sp-times-permutations size set set-size out out-size)
  (status-t sp-time-t sp-time-t* sp-time-t sp-time-t*** sp-time-t*)
  (sp-times-multiplications start factor count out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-additions start summand count out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-select a indices size out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t*)
  (sp-times-bits->times a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (sp-times-shuffle a size) (void sp-time-t* sp-time-t)
  (sp-times-random-binary size out) (status-t sp-time-t sp-time-t*)
  (sp-times-gt-indices a size n out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t* sp-time-t*)
  (sp-times-select-random a size out out-size) (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-constant a size value out) (status-t sp-time-t sp-time-t sp-time-t sp-time-t**)
  (sp-shuffle swap a size) (void (function-pointer void void* size-t size-t) void* size-t)
  (sp-times-scale a a-size factor out) (status-t sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-shuffle-swap a i1 i2) (void void* size-t size-t)
  (sp-samples-smooth a size radius out) (status-t sp-sample-t* sp-time-t sp-time-t sp-sample-t*)
  (sp-times-array-free a size) (void sp-time-t** sp-time-t)
  (sp-samples-array-free a size) (void sp-sample-t** sp-time-t)
  (sp-times-contains a size b) (uint8-t sp-time-t* sp-time-t sp-time-t)
  (sp-times-random-discrete-unique cudist cudist-size size out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-times-sequences base digits size out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-range start end out) (void sp-time-t sp-time-t sp-time-t*)
  (sp-time-round-to-multiple a base) (sp-time-t sp-time-t sp-time-t)
  (sp-times-limit a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (sp-samples-limit-abs a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  sp-times-counted-sequences-t (type (struct (count sp-time-t) (sequence sp-time-t*)))
  (sp-times-counted-sequences-sort-swap a b c) (void void* ssize-t ssize-t)
  (sp-times-counted-sequences-sort-less a b c) (uint8-t void* ssize-t ssize-t)
  (sp-times-counted-sequences-sort-greater a b c) (uint8-t void* ssize-t ssize-t)
  (sp-times-deduplicate a size out out-size) (status-t sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  (sp-times-counted-sequences-add a size width out)
  (void sp-time-t* sp-time-t sp-time-t sp-sequence-hashtable-t)
  (sp-times-counted-sequences-values known min out out-size)
  (void sp-sequence-hashtable-t sp-time-t sp-times-counted-sequences-t* sp-time-t*)
  (sp-times-counted-sequences-count a width b)
  (sp-time-t sp-time-t* sp-time-t sp-sequence-hashtable-t)
  (sp-times-remove in size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-insert-space in size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-subdivide-difference a size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-times-blend a b fraction size out)
  (void sp-time-t* sp-time-t* sp-sample-t sp-time-t sp-time-t*)
  (sp-times-mask a b coefficients size out)
  (void sp-time-t* sp-time-t* sp-sample-t* sp-time-t sp-time-t*)
  (sp-samples-blend a b fraction size out)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-time-t sp-sample-t*)
  (sp-times-make-seamless-right a a-size b b-size out)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t sp-time-t*)
  (sp-times-make-seamless-left a a-size b b-size out)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t sp-time-t*)
  (sp-times-extract-in-range a size min max out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t* sp-time-t*)
  (sp-samples->units a size b) (void sp-sample-t* size-t sp-sample-t*))

(sc-comment "filter")

(pre-define
  sp-filter-state-t sp-convolution-filter-state-t
  sp-filter-state-free sp-convolution-filter-state-free
  (sp-cheap-filter-lp ...) (sp-cheap-filter sp-state-variable-filter-lp __VA_ARGS__)
  (sp-cheap-filter-hp ...) (sp-cheap-filter sp-state-variable-filter-hp __VA_ARGS__)
  (sp-cheap-filter-bp ...) (sp-cheap-filter sp-state-variable-filter-bp __VA_ARGS__)
  (sp-cheap-filter-br ...) (sp-cheap-filter sp-state-variable-filter-br __VA_ARGS__)
  (sp-declare-cheap-filter-state name) (define name sp-cheap-filter-state-t (struct-literal 0)))

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
    (function-pointer void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*))
  (sp-moving-average in in-size prev next radius out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir-length transition) (sp-time-t sp-sample-t)
  (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-sample-t sp-sample-t sp-time-t* sp-sample-t**)
  (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-sample-t sp-sample-t sp-bool-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-bp-br in in-len
    cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-bool-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  (sp-state-variable-filter-lp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-hp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-bp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-br out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-peak out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-all out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-cheap-filter type in in-size cutoff passes q-factor state out)
  (void sp-state-variable-filter-t sp-sample-t*
    sp-time-t sp-sample-t sp-time-t sp-sample-t sp-cheap-filter-state-t* sp-sample-t*)
  (sp-cheap-filter-state-free a) (void sp-cheap-filter-state-t*)
  (sp-cheap-filter-state-new max-size is-multipass out-state)
  (status-t sp-time-t sp-bool-t sp-cheap-filter-state-t*)
  (sp-filter in in-size cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-filter-state-t** sp-sample-t*))

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
  (sp-event-reset x) (set x sp-event-null)
  (sp-declare-event id) (begin (define id sp-event-t (struct-literal 0)) (set id.memory.data 0))
  (sp-declare-event-2 id1 id2) (begin (sp-declare-event id1) (sp-declare-event id2))
  (sp-declare-event-3 id1 id2 id3)
  (begin (sp-declare-event id1) (sp-declare-event id2) (sp-declare-event id3))
  (sp-declare-event-4 id1 id2 id3 id4)
  (begin (sp-declare-event-2 id1 id2) (sp-declare-event-2 id3 id4))
  (sp-declare-group id) (begin (sp-declare-event id) (set id.prepare sp-group-prepare))
  (sp-declare-group-parallel id)
  (begin (sp-declare-event id) (set id.prepare sp-group-prepare-parallel))
  (sp-declare-event-list id) (define id sp-event-list-t* 0)
  (sp-event-duration a) (- a.end a.start)
  (sp-event-duration-set a duration) (set a.end (+ a.start duration))
  (sp-event-move a start) (set a.end (+ start (- a.end a.start)) a.start start)
  sp-group-size-t uint16-t
  (sp-event-memory-add event address) (sp-event-memory-add2 event address free)
  (sp-event-memory-add-2 a data1 data2)
  (begin (sp-event-memory-add a data1) (sp-event-memory-add a data2))
  (sp-event-memory-add-3 a data1 data2 data3)
  (begin (sp-event-memory-add-2 a data1 data2) (sp-event-memory-add a data3))
  sp-sine-config-t sp-wave-event-config-t
  sp-memory-add array3-add
  sp-seq-events-prepare sp-event-list-reverse
  (free-event-on-error event-address) (free-on-error (: event-address free) event-address)
  (free-event-on-exit event-address) (free-on-exit (: event-address free) event-address)
  (sp-group-event-list event) (convert-type (address-of (: event data)) sp-event-list-t**)
  (sp-event-free a) (if a.free (a.free &a))
  (sp-event-pointer-free a) (if a:free (a:free a))
  (sp-define-event name _prepare duration)
  (begin
    "use case: event variables defined at the top-level"
    (define name sp-event-t
      (struct-literal (prepare _prepare) (start 0)
        (end duration) (data 0) (memory (struct-literal 0)))))
  (sp-event-memory-malloc event count type pointer-address)
  (begin
    "allocated memory with malloc, save address in pointer at pointer-address,
     and also immediately add the memory to event memory to be freed with event.free"
    (sp-malloc-type count type pointer-address)
    (sp-event-memory-add _event *pointer-address))
  (sp-event-config-load variable-name type event)
  (define variable-name type (pointer-get (convert-type event:config type*)))
  (sp-sound-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-sound-event-prepare config _config)
  (sp-wave-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-wave-event-prepare config _config)
  (sp-noise-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-noise-event-prepare config _config)
  (sp-cheap-noise-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-cheap-noise-event-prepare config _config)
  (sp-group-event event-pointer) (struct-pointer-set event-pointer prepare sp-group-prepare))

(array3-declare-type sp-memory memreg2-t)

(declare
  sp-memory-free-t (type (function-pointer void void*))
  sp-event-t struct
  sp-event-t
  (type
    (struct
      sp-event-t
      (start sp-time-t)
      (end sp-time-t)
      (generate (function-pointer status-t sp-time-t sp-time-t sp-block-t (struct sp-event-t*)))
      (prepare (function-pointer status-t (struct sp-event-t*)))
      (free (function-pointer void (struct sp-event-t*)))
      (data void*)
      (config void*)
      (memory sp-memory-t)))
  sp-event-generate-t (type (function-pointer status-t sp-time-t sp-time-t sp-block-t sp-event-t*))
  sp-event-list-t
  (type
    (struct
      sp-event-list-struct
      (previous (struct sp-event-list-struct*))
      (next (struct sp-event-list-struct*))
      (event sp-event-t)))
  sp-channel-config-t
  (type
    (struct
      (use sp-bool-t)
      (mute sp-bool-t)
      (delay sp-time-t)
      (phs sp-time-t)
      (amp sp-sample-t)
      (amod sp-sample-t*)))
  sp-wave-event-config-t
  (type
    (struct
      (wvf sp-sample-t*)
      (wvf-size sp-time-t)
      (phs sp-time-t)
      (frq sp-time-t)
      (fmod sp-time-t*)
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-wave-event-state-t
  (type
    (struct
      (wvf sp-sample-t*)
      (wvf-size sp-time-t)
      (phs sp-time-t)
      (frq sp-time-t)
      (fmod sp-time-t*)
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (channel sp-channel-count-t)))
  sp-noise-event-config-t
  (type
    (struct
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (cutl sp-sample-t)
      (cuth sp-sample-t)
      (trnl sp-sample-t)
      (trnh sp-sample-t)
      (cutl-mod sp-sample-t*)
      (cuth-mod sp-sample-t*)
      (resolution sp-time-t)
      (is-reject uint8-t)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-cheap-noise-event-config-t
  (type
    (struct
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (cut sp-sample-t)
      (cut-mod sp-sample-t*)
      (q-factor sp-sample-t)
      (q-factor-mod sp-sample-t*)
      (passes sp-time-t)
      (type sp-state-variable-filter-t)
      (random-state sp-random-state-t*)
      (resolution sp-time-t)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-sound-event-config-t
  (type
    (struct
      (noise sp-bool-t)
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (frq sp-time-t)
      (fmod sp-time-t*)
      (phs sp-time-t)
      (wdt sp-time-t)
      (wmod sp-time-t*)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-event-prepare-t (function-pointer status-t sp-event-t*)
  sp-map-generate-t
  (type (function-pointer status-t sp-time-t sp-time-t sp-block-t sp-block-t void*))
  sp-map-event-state-t
  (type (struct (event sp-event-t) (map-generate sp-map-generate-t) (state void*)))
  sp-map-event-config-t
  (type
    (struct (event sp-event-t) (map-generate sp-map-generate-t) (state void*) (isolate sp-bool-t)))
  (sp-channel-config-zero a) (void sp-channel-config-t*))

(define sp-event-null sp-event-t (struct-literal 0))

(declare
  (sp-event-list-display a) (void sp-event-list-t*)
  (sp-event-list-reverse a) (void sp-event-list-t**)
  (sp-event-list-validate a) (void sp-event-list-t*)
  (sp-event-list-remove-element a element) (void sp-event-list-t** sp-event-list-t*)
  (sp-event-list-add a event) (status-t sp-event-list-t** sp-event-t)
  (sp-event-list-free events) (void sp-event-list-t**)
  (sp-event-memory-ensure a additional-size) (status-t sp-event-t* sp-time-t)
  (sp-event-memory-add2 event address handler) (void sp-event-t* void* sp-memory-free-t)
  (sp-event-memory-free event) (void sp-event-t*)
  (sp-seq start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-seq-parallel start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-wave-event-prepare event) (status-t sp-event-t*)
  (sp-noise-event-prepare event) (status-t sp-event-t*)
  (sp-cheap-noise-event-prepare event) (status-t sp-event-t*)
  (sp-group-prepare event) (status-t sp-event-t*)
  (sp-group-prepare-parallel a) (status-t sp-event-t*)
  (sp-group-add a event) (status-t sp-event-t* sp-event-t)
  (sp-group-append a event) (status-t sp-event-t* sp-event-t)
  (sp-group-add-set group start duration event) (status-t sp-event-t* sp-time-t sp-time-t sp-event-t)
  (sp-group-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-parallel-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-free a) (void sp-event-t*)
  (sp-map-event-prepare event) (status-t sp-event-t*)
  (sp-channel-config mute delay phs amp amod)
  (sp-channel-config-t sp-bool-t sp-time-t sp-time-t sp-sample-t sp-sample-t*)
  (sp-group-free) (void sp-event-t*)
  (sp-wave-event-free) (void sp-event-t*)
  (sp-noise-event-free) (void sp-event-t*)
  (sp-cheap-noise-event-free) (void sp-event-t*)
  (sp-map-event-free) (void sp-event-t*)
  (sp-noise-event-config-new out) (status-t sp-noise-event-config-t**)
  (sp-cheap-noise-event-config-new out) (status-t sp-cheap-noise-event-config-t**)
  (sp-wave-event-config-new out) (status-t sp-wave-event-config-t**)
  (sp-map-event-config-new out) (status-t sp-map-event-config-t**)
  (sp-wave-event-config-defaults config) (void sp-wave-event-config-t*)
  (sp-sound-event-prepare event) (status-t sp-event-t*)
  (sp-sound-event-config-new out) (status-t sp-sound-event-config-t**))

(sc-comment "path")

(pre-define
  sp-path-t spline-path-t
  sp-path-value-t spline-path-value-t
  sp-path-point-t spline-path-point-t
  sp-path-segment-t spline-path-segment-t
  sp-path-segment-count-t spline-path-segment-count-t
  sp-path-line spline-path-line
  sp-path-move spline-path-move
  sp-path-bezier spline-path-bezier
  sp-path-bezier-arc spline-path-bezier-arc
  sp-path-constant spline-path-constant
  sp-path-path spline-path-path
  sp-path-prepare-segments spline-path-prepare-segments
  sp-path-i-line spline-path-i-line
  sp-path-i-move spline-path-i-move
  sp-path-i-bezier spline-path-i-bezier
  sp-path-i-bezier-arc spline-path-i-bezier-arc
  sp-path-i-constant spline-path-i-constant
  sp-path-i-path spline-path-i-path
  sp-path-end spline-path-end
  sp-path-size spline-path-size
  sp-path-free spline-path-free
  sp-path-get spline-path-get
  sp-path-set spline-path-set
  (sp-path-times-constant out size value)
  (sp-path-times-2 out size (sp-path-move 0 value) (sp-path-constant))
  (sp-path-samples-constant out size value)
  (sp-path-samples-2 out size (sp-path-move 0 value) (sp-path-constant))
  (sp-path-curves-config-declare name _segment-count)
  (begin
    (declare
      name sp-path-curves-config-t
      (pre-concat name _x) (array sp-path-value-t _segment-count)
      (pre-concat name _y) (array sp-path-value-t _segment-count)
      (pre-concat name _c) (array sp-path-value-t _segment-count))
    (struct-set name
      segment-count _segment-count
      x (pre-concat name _x)
      y (pre-concat name _y)
      c (pre-concat name _c))))

(declare
  sp-path-curves-config-t
  (type
    (struct
      (segment-count sp-time-t)
      (x sp-path-value-t*)
      (y sp-path-value-t*)
      (c sp-path-value-t*)))
  (sp-path-samples-new path size out) (status-t sp-path-t sp-time-t sp-sample-t**)
  (sp-path-samples-1 out size s1) (status-t sp-sample-t** sp-time-t sp-path-segment-t)
  (sp-path-samples-2 out size s1 s2)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (sp-path-samples-3 out size s1 s2 s3)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-samples-4 out size s1 s2 s3 s4)
  (status-t sp-sample-t** sp-time-t
    sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times-new path size out) (status-t sp-path-t sp-time-t sp-time-t**)
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
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-sample-t** sp-time-t*)
  (sp-path-times-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-time-t** sp-time-t*)
  (sp-path-multiply path x-factor y-factor) (void sp-path-t sp-sample-t sp-sample-t)
  (sp-path-derivations-normalized base count x-changes y-changes out)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-path-t**)
  (sp-path-samples-derivations-normalized path count x-changes y-changes out out-sizes)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-sample-t*** sp-time-t**)
  (sp-path-curves-config-new segment-count out) (status-t sp-time-t sp-path-curves-config-t*)
  (sp-path-curves-config-free a) (void sp-path-curves-config-t)
  (sp-path-curves-new config out) (status-t sp-path-curves-config-t sp-path-t*)
  (sp-path-curves-times-new config length out)
  (status-t sp-path-curves-config-t sp-time-t sp-time-t**)
  (sp-path-curves-samples-new config length out)
  (status-t sp-path-curves-config-t sp-time-t sp-sample-t**))

(sc-comment "statistics")

(declare
  (sp-sequence-max size min-size) (sp-time-t sp-time-t sp-time-t)
  (sp-set-sequence-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (sp-permutations-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (sp-compositions-max sum) (sp-time-t sp-time-t)
  sp-stat-times-f-t (type (function-pointer uint8-t sp-time-t* sp-time-t sp-sample-t*))
  sp-stat-samples-f-t (type (function-pointer uint8-t sp-sample-t* sp-time-t sp-sample-t*))
  (sp-stat-times-range a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-repetition a size width out) (uint8-t sp-time-t* sp-time-t sp-time-t sp-sample-t*)
  (sp-stat-times-repetition-all a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-mean a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-deviation a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-median a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-center a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-inharmonicity a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-kurtosis a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-skewness a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-repetition-all a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-mean a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-deviation a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-inharmonicity a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-median a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-center a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-range a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-kurtosis a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-skewness a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-scale->times a size max out) (void sp-sample-t* sp-time-t sp-time-t sp-time-t*)
  (sp-stat-unique-max size width) (sp-time-t sp-time-t sp-time-t)
  (sp-stat-unique-all-max size) (sp-time-t sp-time-t)
  (sp-stat-repetition-all-max size) (sp-time-t sp-time-t)
  (sp-stat-times-repetition a size width out) (uint8-t sp-time-t* sp-time-t sp-time-t sp-sample-t*)
  (sp-stat-repetition-max size width) (sp-time-t sp-time-t sp-time-t))

(sc-comment "main 2")

(pre-define
  (rt n d)
  (begin
    "return a sample count relative to the current default sample rate sp_rate.
     (rate / d * n)
     example (rt 1 2) returns half of sp_rate"
    (convert-type (* (/ sp-rate d) n) sp-time-t))
  (rts n d)
  (begin
    "like rt but works before sp_initialize has been called"
    (convert-type (* (/ _sp-rate d) n) sp-time-t))
  srq status-require)

(declare
  sp-render-config-t
  (type (struct (channel-count sp-channel-count-t) (rate sp-time-t) (block-size sp-time-t)))
  (sp-render-config channel-count rate block-size)
  (sp-render-config-t sp-channel-count-t sp-time-t sp-time-t)
  (sp-render-file event start end config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*)
  (sp-render-block event start end config out)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t sp-block-t*)
  (sp-render event file-or-plot) (status-t sp-event-t uint8-t))

(pre-include-guard-end)