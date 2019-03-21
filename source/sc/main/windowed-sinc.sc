(sc-comment
  "implementation of a windowed sinc low-pass and high-pass filter for continuous streams of sample arrays.
  sample-rate, radian cutoff frequency and transition band width is variable per call.
  build with the information on https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter")

(define (sp-window-blackman a width) (sp-float-t sp-float-t sp-sample-count-t)
  (return
    (+
      (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1))))) (* 0.08 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-windowed-sinc-lp-hp-ir-length transition) (sp-sample-count-t sp-float-t)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (declare a sp-sample-count-t)
  (set a (ceil (/ 4 transition)))
  (if (not (modulo a 2)) (set a (+ 1 a)))
  (return a))

(define (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-sample-count-t*)
  (set *out-len 1)
  (return (sph-helper-calloc (sizeof sp-sample-t) out-ir)))

(define (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-sample-count-t*)
  status-declare
  (status-require (sph-helper-malloc (sizeof sp-sample-t) out-ir))
  (set
    **out-ir 1
    *out-len 1)
  (label exit
    (return status)))

(define (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-sample-count-t*)
  "create an impulse response kernel for a windowed sinc low-pass or high-pass filter.
  uses a truncated blackman window.
  allocates out-ir, sets out-len.
  cutoff and transition are as fraction 0..0.5 of the sampling rate"
  status-declare
  (declare
    center-index sp-float-t
    i sp-sample-count-t
    ir sp-sample-t*
    len sp-sample-count-t
    sum sp-float-t)
  (set
    len (sp-windowed-sinc-lp-hp-ir-length transition)
    center-index (/ (- len 1.0) 2.0))
  (status-require (sph-helper-malloc (* len (sizeof sp-sample-t)) &ir))
  (sc-comment
    "set the windowed sinc" "nan can be set here if the freq and transition values are invalid")
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (* (sp-window-blackman i len) (sp-sinc (* 2 cutoff (- i center-index))))))
  (sc-comment "scale to get unity gain")
  (set sum (sp-sample-sum ir len))
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (/ (array-get ir i) sum)))
  (if is-high-pass (sp-spectral-inversion-ir ir len))
  (set
    *out-ir ir
    *out-len len)
  (label exit
    (return status)))

(define
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-sample-t** sp-sample-count-t*)
  "like sp-windowed-sinc-ir-lp but for a band-pass or band-reject filter.
  optimisation: if one cutoff is at or above maximum then create only either low-pass or high-pass"
  status-declare
  (declare
    hp-ir sp-sample-t*
    hp-len sp-sample-count-t
    lp-ir sp-sample-t*
    lp-len sp-sample-count-t
    i sp-sample-count-t
    start sp-sample-count-t
    end sp-sample-count-t
    out sp-sample-t*
    over sp-sample-t*)
  (if is-reject
    (begin
      (if (>= 0.0 cutoff-l)
        (if (<= 0.5 cutoff-h) (return (sp-null-ir out-ir out-len))
          (return (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #t out-ir out-len)))
        (if (<= 0.5 cutoff-h)
          (return (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #f out-ir out-len))))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #f &lp-ir &lp-len))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #t &hp-ir &hp-len))
      (sc-comment "prepare to add the shorter ir to the longer one center-aligned")
      (if (> lp-len hp-len)
        (set
          start (- (/ (- lp-len 1) 2) (/ (- hp-len 1) 2))
          end (+ hp-len start)
          out lp-ir
          over hp-ir
          *out-len lp-len)
        (set
          start (- (/ (- hp-len 1) 2) (/ (- lp-len 1) 2))
          end (+ lp-len start)
          out hp-ir
          over lp-ir
          *out-len hp-len))
      (sc-comment "sum lp and hp ir samples")
      (for ((set i start) (< i end) (set i (+ 1 i)))
        (set (array-get out i) (+ (array-get over (- i start)) (array-get out i))))
      (free over)
      (set *out-ir out))
    (begin
      (sc-comment "meaning of cutoff high/low is switched.")
      (if (>= 0.0 cutoff-l)
        (if (<= 0.5 cutoff-h) (return (sp-passthrough-ir out-ir out-len))
          (return (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #f out-ir out-len)))
        (if (<= 0.5 cutoff-h)
          (return (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #t out-ir out-len))))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #t &hp-ir &hp-len))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #f &lp-ir &lp-len))
      (sc-comment "convolve lp and hp ir samples")
      (set *out-len (- (+ lp-len hp-len) 1))
      (status-require (sph-helper-calloc (* *out-len (sizeof sp-sample-t)) out-ir))
      (sp-convolve-one lp-ir lp-len hp-ir hp-len *out-ir)))
  (label exit
    (return status)))

(define (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len)
  (status-t void* sp-sample-t** sp-sample-count-t*)
  "maps arguments from the generic ir-f-arguments array.
  arguments is (sp-float-t:cutoff sp-float-t:transition boolean:is-high-pass)"
  (declare
    cutoff sp-float-t
    transition sp-float-t
    is-high-pass boolean)
  (set
    cutoff (pointer-get (convert-type arguments sp-float-t*))
    transition (pointer-get (+ 1 (convert-type arguments sp-float-t*)))
    is-high-pass (pointer-get (convert-type (+ 2 (convert-type arguments sp-float-t*)) boolean*)))
  (return (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)))

(define (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len)
  (status-t void* sp-sample-t** sp-sample-count-t*)
  "maps arguments from the generic ir-f-arguments array.
  arguments is (sp-float-t:cutoff-l sp-float-t:cutoff-h sp-float-t:transition boolean:is-reject)"
  (declare
    cutoff-l sp-float-t
    cutoff-h sp-float-t
    transition-l sp-float-t
    transition-h sp-float-t
    is-reject boolean)
  (set
    cutoff-l (pointer-get (convert-type arguments sp-float-t*))
    cutoff-h (pointer-get (+ 1 (convert-type arguments sp-float-t*)))
    transition-l (pointer-get (+ 2 (convert-type arguments sp-float-t*)))
    transition-h (pointer-get (+ 3 (convert-type arguments sp-float-t*)))
    is-reject (pointer-get (convert-type (+ 4 (convert-type arguments sp-float-t*)) boolean*)))
  (return
    (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)))

(define (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  "a windowed sinc low-pass or high-pass filter for segments of continuous streams with
  variable sample-rate, frequency, transition and impulse response type per call.
  * cutoff: as a fraction of the sample rate, 0..0.5
  * transition: like cutoff
  * is-high-pass: if true then it will reduce low frequencies
  * out-state: if zero then state will be allocated.
  * out-samples: owned by the caller. length must be at least in-len"
  status-declare
  (declare
    a (array uint8-t ((+ (sizeof boolean) (* 2 (sizeof sp-float-t)))))
    a-len uint8-t)
  (sc-comment "set arguments array for ir-f")
  (set
    a-len (+ (sizeof boolean) (* 2 (sizeof sp-float-t)))
    (pointer-get (convert-type a sp-float-t*)) cutoff
    (pointer-get (+ 1 (convert-type a sp-float-t*))) transition
    (pointer-get (convert-type (+ 2 (convert-type a sp-float-t*)) boolean*)) is-high-pass)
  (sc-comment "apply filter")
  (status-require
    (sp-convolution-filter in in-len sp-windowed-sinc-lp-hp-ir-f a a-len out-state out-samples))
  (label exit
    (return status)))

(define
  (sp-windowed-sinc-bp-br
    in in-len cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  "like sp-windowed-sinc-lp-hp but for a band-pass or band-reject filter"
  status-declare
  (declare
    a (array uint8-t ((+ (sizeof boolean) (* 3 (sizeof sp-float-t)))))
    a-len uint8-t)
  (sc-comment "set arguments array for ir-f")
  (set
    a-len (+ (sizeof boolean) (* 4 (sizeof sp-float-t)))
    (pointer-get (convert-type a sp-float-t*)) cutoff-l
    (pointer-get (+ 1 (convert-type a sp-float-t*))) cutoff-h
    (pointer-get (+ 2 (convert-type a sp-float-t*))) transition-l
    (pointer-get (+ 3 (convert-type a sp-float-t*))) transition-h
    (pointer-get (convert-type (+ 4 (convert-type a sp-float-t*)) boolean*)) is-reject)
  (sc-comment "apply filter")
  (status-require
    (sp-convolution-filter in in-len sp-windowed-sinc-bp-br-ir-f a a-len out-state out-samples))
  (label exit
    (return status)))