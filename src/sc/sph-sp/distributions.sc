(pre-define (distributions-template value-t value-type-name type-name subtract abs)
  (begin
    (define ((pre-concat sp_ type-name _additions) start summand count out)
      (void value-t value-t value-t value-t*)
      "write count cumulative additions with summand from start to out.
       with summand, only the nth additions are written.
       use case: generating harmonic frequency values"
      (for ((define i sp-size-t 0) (< i count) (set+ i 1 start summand))
        (set (array-get out i) start)))
    (define ((pre-concat sp_ type-name _linear_decreasing) base k count out)
      (void value-t value-t value-t value-t*)
      "a(n) = base – k·n"
      (sp-for-each-index i count (set (array-get out i) (- base (* k i)))))
    (define ((pre-concat sp_ type-name _linear-series) base offset spacing count out)
      (void value-t value-t value-t sp-time-t value-t*)
      (sp-for-each-index i count (set (array-get out i) (* base (+ offset (* spacing i))))))
    (define ((pre-concat sp_ type-name _odd-harmonics) base count out)
      (void value-t sp-time-t value-t*)
      "f(n) = base * (2·n+1)"
      (sp-for-each-index i count (set (array-get out i) (* base (+ 1 (* 2 i))))))
    (define ((pre-concat sp_ type-name _even-harmonics) base count out)
      (void value-t sp-time-t value-t*)
      "f(n) = base * 2·(n+1)"
      (sp-for-each-index i count (set (array-get out i) (* base 2 (+ 1 i)))))
    (define ((pre-concat sp_ type-name _nth-harmonics) base k count out)
      (void value-t value-t sp-time-t value-t*)
      "f(n) = base * k * (n+1)"
      (sp-for-each-index i count (set (array-get out i) (* base k (+ 1 i)))))
    (define ((pre-concat sp_ type-name _cumulative) base deltas count out)
      (void value-t value-t* sp-time-t value-t*)
      "f(n) = f(n–1) + deltas[n]"
      (define acc value-t base)
      (sp-for-each-index i count (set acc (+ acc (array-get deltas i))) (set (array-get out i) acc)))
    (define ((pre-concat sp_ type-name _decumulative) base deltas count out)
      (void value-t value-t* sp-time-t value-t*)
      "f(n) = f(n–1) – deltas[n]"
      (define acc value-t base)
      (sp-for-each-index i count (set acc (- acc (array-get deltas i))) (set (array-get out i) acc)))
    (define ((pre-concat sp_ type-name _prime-indexed) base count out)
      (void value-t sp-time-t value-t*)
      "f(n) = base * pₙ, pₙ = nth prime"
      (sp-for-each-index i (sp-time-min count (/ (sizeof sp-primes) (sizeof sp-time-t)))
        (set (array-get out i) (* base (array-get sp-primes i)))))
    (define ((pre-concat sp_ type-name _modular-series) base mod delta count out)
      (void value-t sp-time-t sp-sample-t sp-time-t value-t*)
      "f(n) = base + (n mod mod)·delta"
      (sp-for-each-index i count (set (array-get out i) (+ base (* (fmod i mod) delta)))))
    (define ((pre-concat sp_ type-name _fixed_sets) base ratios len out)
      (void value-t value-t* value-t value-t*)
      "f(n) = base * ratios[n]"
      (sp-for-each-index i len (set (array-get out i) (* base (array-get ratios i)))))
    (define ((pre-concat sp_ type-name _clustered) center spread count out)
      (void value-t value-t value-t value-t*)
      "f(n) = center + spread·(n–(count–1)/2)"
      (define half value-t (/ (- count 1) 2))
      (sp-for-each-index i count (set (array-get out i) (+ center (* spread (- i half))))))
    (define ((pre-concat sp_ type-name _exponential) base k count out)
      (void value-t value-t value-t value-t*)
      "a(n) = base·e^(–k·n)"
      (sp-for-each-index i count (set (array-get out i) (* base (exp (- (* k i)))))))
    (define ((pre-concat sp_ type-name _gaussian) base centre width count out)
      (void value-t value-t value-t value-t value-t*)
      "a(n) = base·e^(–((n–centre)²)/(2·width²))"
      (define denom value-t (* 2 (* width width)))
      (sp-for-each-index i count
        (define d value-t (- i centre))
        (set (array-get out i) (* base (exp (/ (- (* d d)) denom))))))
    (define ((pre-concat sp_ type-name _power) base p count out)
      (void value-t value-t value-t value-t*)
      "a(n) = base·(n+1)^(–p)"
      (sp-for-each-index i count (set (array-get out i) (* base (pow (+ 1 i) (- p))))))
    (define ((pre-concat sp_ type-name _bessel) base count out) (void value-t value-t value-t*)
      "a(n) = base·j0(n)"
      (sp-for-each-index i count (set (array-get out i) (* base (j0 i)))))
    (define ((pre-concat sp_ type-name _logistic) base k count out)
      (void value-t value-t value-t value-t*)
      "a(n) = base / (1 + e^(k·(n–mid)))"
      (define mid value-t (/ (- count 1) 2))
      (sp-for-each-index i count (set (array-get out i) (/ base (+ 1 (exp (* k (- i mid))))))))))

(distributions-template sp-time-t time times sp-inline-no-underflow-subtract sp-inline-abs)
(distributions-template sp-sample-t sample samples sp-subtract fabs)

(define (sp-times-geometric base ratio count out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  (sp-for-each-index i count
    (define r-pow sp-time-t 1)
    (sp-for-each-index j i (set* r-pow ratio))
    (set (array-get out i) (* base r-pow))))

(define (sp-times-logarithmic base scale count out)
  (void sp-time-t sp-sample-t sp-time-t sp-time-t*)
  (sp-for-each-index i count
    (define v sp-sample-t (* base (log1p (* scale (+ i 1)))))
    (set (array-get out i) (llround v))))

(define (sp-samples-geometric base ratio count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-for-each-index i count (set (array-get out i) (* base (pow ratio i)))))

(define (sp-samples-logarithmic base scale count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-for-each-index i count (set (array-get out i) (* base (log1p (* scale (+ i 1)))))))

(define (sp-samples-beta-distribution base alpha beta-param count out)
  (void sp-sample-t sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (define max-val sp-sample-t 0)
  (sp-for-each-index i count
    (define
      x sp-sample-t (/ i (- count 1))
      val sp-sample-t (* (pow x (- alpha 1)) (pow (- 1 x) (- beta-param 1))))
    (set (array-get out i) val)
    (if (> val max-val) (set max-val val)))
  (define scale sp-sample-t (/ base max-val))
  (sp-for-each-index i count (set (array-get out i) (* (array-get out i) scale))))

(define (sp-samples-binary-mask base pattern pattern-len count out)
  (void sp-sample-t uint8-t* sp-time-t sp-time-t sp-sample-t*)
  (sp-for-each-index i count
    (define mask-val sp-sample-t (if* (array-get pattern (modulo i pattern-len)) base 0))
    (set (array-get out i) mask-val)))

(define (sp-samples-segment-steps base levels segments count out)
  (void sp-sample-t sp-sample-t* sp-time-t sp-time-t sp-sample-t*)
  (define
    segment-len sp-time-t (/ count segments)
    remainder sp-time-t (modulo count segments)
    idx sp-time-t 0)
  (sp-for-each-index s segments
    (define len sp-time-t (+ segment-len (if* (< s remainder) 1 0)))
    (sp-for-each-index j len
      (set (array-get out idx) (* base (array-get levels s)))
      (set idx (+ idx 1)))))

(define (sp-times-random-discrete-unique cudist cudist-size size out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "create size number of unique discrete random numbers with the distribution given by cudist"
  (declare a sp-time-t remaining sp-time-t)
  (set remaining (sp-inline-min size cudist-size))
  (while remaining
    (set a (sp-time-random-discrete cudist cudist-size))
    (if (sp-times-contains out (- size remaining) a) continue)
    (set (array-get out (- size remaining)) a)
    (set- remaining 1)))

(define (sp-times-random-binary size out) (status-t sp-time-t sp-time-t*)
  "write to out values that are randomly either 1 or 0"
  (declare random-size sp-time-t temp sp-time-t*)
  status-declare
  (set random-size
    (if* (< size (* (sizeof sp-time-t) 8)) 1 (+ (/ size (* (sizeof sp-time-t) 8)) 1)))
  (status-require (sp-times-new random-size &temp))
  (sp-times-random random-size temp)
  (sp-times-bits->times temp size out)
  (free temp)
  (label exit status-return))

(define (sp-sample-random-discrete-bounded cudist cudist-size range)
  (sp-sample-t sp-time-t* sp-time-t sp-sample-t)
  (return
    (* range
      (/ (sp-time-random-discrete cudist cudist-size) (convert-type cudist-size sp-sample-t)))))