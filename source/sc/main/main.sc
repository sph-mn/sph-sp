(pre-include
  "stdio.h"
  "fcntl.h"
  "sndfile.h"
  "../main/sph-sp.h"
  "../foreign/sph/float.c"
  "../foreign/sph/helper.c" "../foreign/sph/memreg.c" "foreign/nayuki-fft/fft.c")

(pre-define
  sp-status-declare (status-declare-group sp-status-group-sp)
  (sp-libc-status-require-id id) (if (< id 0) (status-set-both-goto sp-status-group-libc id))
  (sp-libc-status-require expression)
  (begin
    (set status.id expression)
    (if (< status.id 0) (status-set-group-goto sp-status-group-libc)
      status-reset))
  (define-sp-interleave name type body)
  (begin
    "define a deinterleave, interleave or similar routine.
    a: source
    b: target"
    (define (name a b a-size channel-count)
      (void type** type* sp-sample-count-t sp-channel-count-t)
      (declare
        b-size sp-sample-count-t
        channel sp-channel-count-t)
      (set b-size (* a-size channel-count))
      (while a-size
        (set
          a-size (- a-size 1)
          channel channel-count)
        (while channel
          (set
            channel (- channel 1)
            b-size (- b-size 1))
          body)))))

(define-sp-interleave
  sp-interleave
  sp-sample-t
  (compound-statement (set (array-get b b-size) (array-get (array-get a channel) a-size))))

(define-sp-interleave
  sp-deinterleave
  sp-sample-t
  (compound-statement (set (array-get (array-get a channel) a-size) (array-get b b-size))))

(define (debug-display-sample-array a len) (void sp-sample-t* sp-sample-count-t)
  "display a sample array in one line"
  (declare i sp-sample-count-t)
  (printf "%.17g" (array-get a 0))
  (for ((set i 1) (< i len) (set i (+ 1 i)))
    (printf " %.17g" (array-get a i)))
  (printf "\n"))

(define (sp-status-description a) (uint8-t* status-t)
  "get a string description for a status id in a status-t"
  (declare b char*)
  (cond
    ( (not (strcmp sp-status-group-sp a.group))
      (case = a.id
        (sp-status-id-eof (set b "end of file"))
        (sp-status-id-input-type (set b "input argument is of wrong type"))
        (sp-status-id-not-implemented (set b "not implemented"))
        (sp-status-id-memory (set b "memory allocation error"))
        (sp-status-id-file-incompatible
          (set b "file channel count or sample rate is different from what was requested"))
        (sp-status-id-file-incomplete (set b "incomplete write"))
        (else (set b ""))))
    ( (not (strcmp sp-status-group-sndfile a.group))
      (set b (convert-type (sf-error-number a.id) char*)))
    ((not (strcmp sp-status-group-sph a.group)) (set b (sph-helper-status-description a)))
    (else (set b "")))
  (return b))

(define (sp-status-name a) (uint8-t* status-t)
  "get a single word identifier for a status id in a status-t"
  (declare b char*)
  (cond
    ( (= 0 (strcmp sp-status-group-sp a.group))
      (case = a.id
        (sp-status-id-input-type (set b "input-type"))
        (sp-status-id-not-implemented (set b "not-implemented"))
        (sp-status-id-memory (set b "memory"))
        (else (set b "unknown"))))
    ((= 0 (strcmp sp-status-group-sndfile a.group)) (set b "sndfile")) (else (set b "unknown")))
  (return b))

(define (sp-block-free a channel-count) (void sp-sample-t** sp-channel-count-t)
  (while channel-count
    (set channel-count (- channel-count 1))
    (free (array-get a channel-count)))
  (free a))

(define (sp-block-alloc channel-count sample-count result)
  (status-t sp-channel-count-t sp-sample-count-t sp-sample-t***)
  "return a newly allocated array for channels with data arrays for each channel"
  status-declare
  (memreg-init (+ channel-count 1))
  (declare
    channel sp-sample-t*
    a sp-sample-t**)
  (status-require (sph-helper-malloc (* channel-count (sizeof sp-sample-t*)) &a))
  (memreg-add a)
  (while channel-count
    (set channel-count (- channel-count 1))
    (status-require (sph-helper-calloc (* sample-count (sizeof sp-sample-t)) &channel))
    (memreg-add channel)
    (set (array-get a channel-count) channel))
  (set *result a)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

(define (sp-sin-lq a) (sp-sample-t sp-float-t)
  "lower precision version of sin() that should be faster"
  (declare
    b sp-sample-t
    c sp-sample-t)
  (set
    b (/ 4 M_PI)
    c (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (sp-float-t sp-float-t)
  "the normalised sinc function"
  (return
    (if* (= 0 a) 1
      (/ (sin (* M_PI a)) (* M_PI a)))))

(define (sp-fft input-len input/output-real input/output-imag)
  (status-t sp-sample-count-t double* double*)
  "all arrays should be input-len and are managed by the caller"
  sp-status-declare
  (status-id-require (not (Fft_transform input/output-real input/output-imag input-len)))
  (label exit
    (return status)))

(define (sp-ffti input-len input/output-real input/output-imag)
  (status-t sp-sample-count-t double* double*)
  "[[real, imaginary], ...]:complex-numbers -> real-numbers
  input-length > 0
  output-length = input-length
  output is allocated and owned by the caller"
  sp-status-declare
  (status-id-require
    (not (= 1 (Fft_inverseTransform input/output-real input/output-imag input-len))))
  (label exit
    (return status)))

(pre-define
  (max a b)
  (if* (> a b) a
    b)
  (min a b)
  (if* (< a b) a
    b))

(define
  (sp-moving-average in in-end in-window in-window-end prev prev-end next next-end radius out)
  (status-t
    sp-sample-t*
    sp-sample-t*
    sp-sample-t*
    sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-count-t sp-sample-t*)
  "apply a centered moving average filter to samples between in-window and in-window-end inclusively and write to out.
   removes high frequencies with little distortion in the time domain but with a long transition.
   all memory is managed by the caller.
   * prev and next can be null pointers if not available
   * zero is used for unavailable values
   * rounding errors are kept low by using modified kahan neumaier summation"
  status-declare
  (declare
    in-left sp-sample-t*
    in-right sp-sample-t*
    outside sp-sample-t*
    sums (array sp-sample-t (3))
    outside-count sp-sample-count-t
    in-missing sp-sample-count-t
    width sp-sample-count-t)
  (set width (+ 1 radius radius))
  (while (<= in-window in-window-end)
    (set
      (array-get sums 0) 0
      (array-get sums 1) 0
      (array-get sums 2) 0
      in-left (max in (- in-window radius))
      in-right (min in-end (+ in-window radius))
      (array-get sums 1) (sp-sample-sum in-left (+ 1 (- in-right in-left))))
    ;(printf "current center: %f\n" (pointer-get in-window))
    ;(printf "in: ")
    ;(debug-display-sample-array in-left (+ 1 (- in-right in-left)))
    (if (and (< (- in-window in-left) radius) prev)
      (begin
        (set
          in-missing (- radius (- in-window in-left))
          outside (max prev (- prev-end in-missing))
          outside-count (- prev-end outside)
          (array-get sums 0) (sp-sample-sum outside outside-count))
        ;(printf "prev: ")
        ;(debug-display-sample-array outside outside-count)
        ))
    (if (and (< (- in-right in-window) radius) next)
      (begin
        (set
          in-missing (- radius (- in-right in-window))
          outside next
          outside-count (min (- next-end next) in-missing)
          (array-get sums 2) (sp-sample-sum outside outside-count))
        ;(printf "next: ")
        ;(debug-display-sample-array outside outside-count)
        ))
    (set
      (pointer-get out) (/ (sp-sample-sum sums 3) width)
      out (+ 1 out)
      in-window (+ 1 in-window)))
  (return status))

(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (declare
    center sp-sample-count-t
    i sp-sample-count-t)
  (for ((set i 0) (< i a-len) (set i (+ 1 i)))
    (set (array-get a i) (* -1 (array-get a i))))
  (set
    center (/ (- a-len 1) 2)
    (array-get a center) (+ 1 (array-get a center))))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1)
    (set
      a-len (- a-len 2)
      (array-get a a-len) (* -1 (array-get a a-len)))))

(define (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  "discrete linear convolution.
  result-samples must be all zeros, its length must be at least a-len + b-len - 1.
  result-samples is owned and allocated by the caller"
  (declare
    a-index sp-sample-count-t
    b-index sp-sample-count-t)
  (set
    a-index 0
    b-index 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set
        (array-get result-samples (+ a-index b-index))
        (+
          (array-get result-samples (+ a-index b-index))
          (* (array-get a a-index) (array-get b b-index)))
        b-index (+ 1 b-index)))
    (set
      b-index 0
      a-index (+ 1 a-index))))

(define (sp-convolve a a-len b b-len carryover-len result-carryover result-samples)
  (void
    sp-sample-t*
    sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-count-t sp-sample-t* sp-sample-t*)
  "discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a-len) to result-samples
  using (b, b-len) as the impulse response. b-len must be greater than zero.
  all heap memory is owned and allocated by the caller.
  result-samples length is a-len.
  result-carryover is previous carryover or an empty array.
  result-carryover length must at least b-len - 1.
  continuous processing does not work correctly if result-samples is smaller than b-len - 1, in this case
  result-carryover will contain values after index a-len - 1 that will not be carried over to the next call.
  carryover-len should be zero for the first call or its content should be zeros.
  carryover-len for subsequent calls should be b-len - 1 or if b-len changed b-len - 1  from the previous call.
  if b-len is one then there is no carryover.
  if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call.
  carryover is the extension of result-samples for generated values that dont fit"
  (declare
    size sp-sample-count-t
    a-index sp-sample-count-t
    b-index sp-sample-count-t
    c-index sp-sample-count-t)
  (if carryover-len
    (if (<= carryover-len a-len)
      (begin
        (sc-comment "copy all entries to result and reset")
        (memcpy result-samples result-carryover (* carryover-len (sizeof sp-sample-t)))
        (memset result-carryover 0 (* carryover-len (sizeof sp-sample-t)))
        (memset (+ carryover-len result-samples) 0 (* (- a-len carryover-len) (sizeof sp-sample-t))))
      (begin
        (sc-comment
          "carryover is larger. set result-samples to all carryover entries that fit."
          "shift remaining carryover to the left")
        (memcpy result-samples result-carryover (* a-len (sizeof sp-sample-t)))
        (memmove
          result-carryover
          (+ a-len result-carryover) (* (- carryover-len a-len) (sizeof sp-sample-t)))
        (memset (+ (- carryover-len a-len) result-carryover) 0 (* a-len (sizeof sp-sample-t)))))
    (memset result-samples 0 (* a-len (sizeof sp-sample-t))))
  (sc-comment "result values." "first process values that dont lead to carryover")
  (set size
    (if* (< a-len b-len) 0
      (- a-len (- b-len 1))))
  (if size (sp-convolve-one a size b b-len result-samples))
  (sc-comment "process values with carryover")
  (for ((set a-index size) (< a-index a-len) (set a-index (+ 1 a-index)))
    (for ((set b-index 0) (< b-index b-len) (set b-index (+ 1 b-index)))
      (set c-index (+ a-index b-index))
      (if (< c-index a-len)
        (set (array-get result-samples c-index)
          (+ (array-get result-samples c-index) (* (array-get a a-index) (array-get b b-index))))
        (set
          c-index (- c-index a-len)
          (array-get result-carryover c-index)
          (+ (array-get result-carryover c-index) (* (array-get a a-index) (array-get b b-index))))))))

(define (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (if (not state) return)
  (free state:ir)
  (free state:carryover)
  (free state:ir-f-arguments)
  (free state))

(define (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  "create or update a previously created state object.
  impulse response array properties are calculated with ir-f using ir-f-arguments.
  eventually frees state.ir.
  the state object is used to store the impulse response, the parameters that where used to create it and
  overlapping data that has to be carried over between calls.
  ir-f-arguments can be stack allocated and will be copied to state on change"
  status-declare
  (declare
    carryover sp-sample-t*
    carryover-alloc-len sp-sample-count-t
    ir sp-sample-t*
    ir-len sp-sample-count-t
    state sp-convolution-filter-state-t*)
  (memreg-init 2)
  (sc-comment
    "create state if not exists. re-use if exists and return early if ir needs not be updated")
  (if *out-state
    (begin
      (sc-comment "existing")
      (set state *out-state)
      (if
        (and
          (= state:ir-f ir-f)
          (= ir-f-arguments-len state:ir-f-arguments-len)
          (= 0 (memcmp state:ir-f-arguments ir-f-arguments ir-f-arguments-len)))
        (begin
          (sc-comment "unchanged")
          (return status))
        (begin
          (sc-comment "changed")
          (if (> ir-f-arguments-len state:ir-f-arguments-len)
            (status-require (sph-helper-realloc ir-f-arguments-len &state:ir-f-arguments)))
          (if state:ir (free state:ir)))))
    (begin
      (sc-comment "new")
      (status-require (sph-helper-malloc (sizeof sp-convolution-filter-state-t) &state))
      (status-require (sph-helper-malloc ir-f-arguments-len &state:ir-f-arguments))
      (memreg-add state)
      (memreg-add state:ir-f-arguments)
      (set
        state:carryover-alloc-len 0
        state:carryover-len 0
        state:carryover 0
        state:ir-f ir-f
        state:ir-f-arguments-len ir-f-arguments-len)))
  (memcpy state:ir-f-arguments ir-f-arguments ir-f-arguments-len)
  (status-require (ir-f ir-f-arguments &ir &ir-len))
  (sc-comment
    "eventually extend carryover array. the array is never shrunk."
    "carryover-len is at least ir-len - 1."
    "carryover-alloc-len is the length of the whole array."
    "new and extended areas must be set to zero")
  (set carryover-alloc-len (- ir-len 1))
  (if state:carryover
    (begin
      (set carryover state:carryover)
      (if (> ir-len state:ir-len)
        (begin
          (if (> carryover-alloc-len state:carryover-alloc-len)
            (begin
              (status-require
                (sph-helper-realloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
              (set state:carryover-alloc-len carryover-alloc-len)))
          (sc-comment "in any case reset the extension area")
          (memset
            (+ (- state:ir-len 1) carryover) 0 (* (- ir-len state:ir-len) (sizeof sp-sample-t))))))
    (begin
      (status-require (sph-helper-calloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
      (set state:carryover-alloc-len carryover-alloc-len)))
  (set
    state:carryover carryover
    state:ir ir
    state:ir-len ir-len
    *out-state state)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

(define
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  "convolute samples \"in\", which can be a segment of a continuous stream, with an impulse response
  kernel created by ir-f with ir-f-arguments.
  ir-f is only used when ir-f-arguments changed.
  values that need to be carried over with calls are kept in out-state.
  * out-state: if zero then state will be allocated. owned by caller
  * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len"
  status-declare
  (declare carryover-len sp-sample-count-t)
  (set carryover-len
    (if* *out-state (- (: *out-state ir-len) 1)
      0))
  (sc-comment "create/update the impulse response kernel")
  (status-require
    (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state))
  (sc-comment "convolve")
  (sp-convolve
    in
    in-len (: *out-state ir) (: *out-state ir-len) carryover-len (: *out-state carryover) out-samples)
  (label exit
    (return status)))

(define (sp-sine-table-new out size) (status-t sp-sample-t** sp-sample-count-t)
  status-declare
  (declare
    i sp-sample-count-t
    out-array sp-sample-t*)
  (status-require (sph-helper-malloc (* size (sizeof sp-sample-t*)) &out-array))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out-array i) (sin (* i (/ M_PI (/ size 2))))))
  (set *out out-array)
  (label exit
    (return status)))

(define (sp-initialise) status-t (return (sp-sine-table-new &sp-sine-48-table 48000)))

(define (sp-cheap-phase-48 current change) (sp-sample-count-t sp-sample-count-t sp-sample-count-t)
  (declare result sp-sample-count-t)
  (set result (+ current change))
  (return
    (if* (<= 48000 result) (modulo result 48000)
      result)))

(define (sp-cheap-phase-48-float current change) (sp-sample-count-t sp-sample-count-t double)
  (return
    (sp-cheap-phase-48 current
      (if* (> 1.0 change) 1
        (sp-cheap-round-positive change)))))

(define (sp-fm-synth out channels start duration config-len config state)
  (status-t
    sp-sample-t**
    sp-sample-count-t
    sp-sample-count-t
    sp-sample-count-t sp-fm-synth-count-t sp-fm-synth-operator-t* sp-sample-count-t**)
  "modifiers must come after carriers in config.
  config-len must not change between calls with the same state.
  sp-initialise must have been called once before.
   modulators can be modulated themselves in chains.
  features: changing amplitude and wavelength per operator per channel, phase offset per operator and channel.
  state is allocated if zero and owned by caller"
  status-declare
  (declare
    amp sp-sample-t
    carrier sp-sample-t*
    channel-i sp-sample-count-t
    i sp-sample-count-t
    modulated-wvl sp-sample-t
    modulation-index (array sp-sample-t* (sp-fm-synth-operator-limit sp-fm-synth-channel-limit))
    modulation sp-sample-t**
    op-i sp-fm-synth-count-t
    op sp-fm-synth-operator-t
    phases sp-sample-count-t*
    phs sp-sample-count-t
    wvl sp-sample-count-t)
  (sc-comment
    "modulation blocks (channel array + data. at least one is carrier and writes only to output)")
  (memreg-init (* (- config-len 1) (+ 1 channels)))
  (sc-comment
    "create new state which contains the initial phase offsets per channel."
    "((phase-offset:channel ...):operator ...)")
  (if (not *state)
    (begin
      (status-require
        (sph-helper-calloc (* channels config-len (sizeof sp-sample-count-t)) &phases))
      (for ((set i 0) (< i config-len) (set i (+ 1 i)))
        (for ((set channel-i 0) (< channel-i channels) (set channel-i (+ 1 channel-i)))
          (set (array-get phases (+ channel-i (* channels i)))
            (array-get (struct-get (array-get config i) phase-offset) channel-i))))))
  (set op-i config-len)
  (sc-comment "the modulation-index contains modulator output")
  (while op-i
    (set
      op-i (- op-i 1)
      op (array-get config op-i)
      modulation (array-get modulation-index op-i))
    (printf "op-i %lu\n" op-i)
    (if op.modifies
      (begin
        (printf "modifies\n" op-i)
        (sc-comment "allocate block for modulator output")
        (status-require (sph-helper-malloc (* channels (sizeof sp-sample-t*)) &carrier))
        (memreg-add carrier)
        (for ((set channel-i 0) (< channel-i channels) (set channel-i (+ 1 channel-i))))
        (sc-comment "generate samples")
        (for ((set channel-i 0) (< channel-i channels) (set channel-i (+ 1 channel-i)))
          (status-require (sph-helper-calloc (* duration (sizeof sp-sample-t)) &carrier))
          (memreg-add carrier)
          (set phs (array-get phases (+ channel-i (* channels op-i))))
          (for ((set i 0) (< i duration) (set i (+ 1 i)))
            (set
              amp (array-get op.amplitude channel-i (+ start i))
              (array-get carrier i) (+ (array-get carrier i) (* amp (sp-sine-48 phs)))
              wvl (array-get op.wavelength channel-i (+ start i))
              modulated-wvl
              (if* modulation (+ wvl (* wvl (array-get modulation channel-i i)))
                wvl)
              phs (sp-cheap-phase-48-float phs (/ 24000 modulated-wvl))))
          (set
            (array-get phases (+ channel-i (* channels op-i))) phs
            (array-get modulation-index (- op.modifies 1) channel-i) carrier)))
      (for ((set channel-i 0) (< channel-i channels) (set channel-i (+ 1 channel-i)))
        (printf "not modifies\n" op-i)
        (set phs (array-get phases (+ channel-i (* channels op-i))))
        (for ((set i 0) (< i duration) (set i (+ 1 i)))
          (set
            amp (array-get op.amplitude channel-i (+ start i))
            wvl (array-get op.wavelength channel-i (+ start i))
            modulated-wvl
            (if* modulation (+ wvl (* wvl (array-get modulation channel-i i)))
              wvl)
            phs (sp-cheap-phase-48-float phs (/ 24000 modulated-wvl))
            (array-get out channel-i i) (+ (array-get out channel-i i) (* amp (sp-sine-48 phs)))))
        (set (array-get phases (+ channel-i (* channels op-i))) phs))))
  (printf "after op processing\n" op-i)
  (set *state phases)
  (label exit
    memreg-free
    (return status)))

(pre-include "../main/windowed-sinc.c" "../main/io.c")