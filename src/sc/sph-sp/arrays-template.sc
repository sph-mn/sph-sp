(sc-comment "functions that work the same for sp-sample-t and sp-time-t")

(pre-define (arrays-template value-t value-type-name type-name subtract abs)
  (begin
    (define ((pre-concat sp_ value-type-name _sort-swap) a b c) (void void* ssize-t ssize-t)
      (declare d value-t)
      (set
        d (array-get (convert-type a value-t*) b)
        (array-get (convert-type a value-t*) b) (array-get (convert-type a value-t*) c)
        (array-get (convert-type a value-t*) c) d))
    (define ((pre-concat sp_ value-type-name _sort-less) a b c) (uint8-t void* ssize-t ssize-t)
      (return (< (array-get (convert-type a value-t*) b) (array-get (convert-type a value-t*) c))))
    (define ((pre-concat sp_ value-type-name _round-to-multiple) a base) (value-t value-t value-t)
      "round to the next integer multiple of base "
      (return
        (if* (= 0 a) base (sp-cheap-round-positive (* (/ a (convert-type base sp-sample-t)) base)))))
    (define ((pre-concat sp_ type-name _min) in count) (value-t value-t* sp-size-t)
      "count must be greater than zero"
      (define out value-t (array-get in 0))
      (for ((define i sp-size-t 1) (< i count) (set+ i 1))
        (if (< (array-get in i) out) (set out (array-get in i))))
      (return out))
    (define ((pre-concat sp_ type-name _max) in count) (value-t value-t* sp-size-t)
      "count must be greater than zero"
      (define out value-t (array-get in 0))
      (for ((define i sp-size-t 1) (< i count) (set+ i 1))
        (if (> (array-get in i) out) (set out (array-get in i))))
      (return out))
    (define ((pre-concat sp_ type-name _absolute-max) in count) (value-t value-t* sp-size-t)
      "get the maximum value in samples array, disregarding sign"
      (declare temp sp-time-t max sp-time-t)
      (set max 0)
      (sp-for-each-index i count
        (set temp (sp-inline-abs (array-get in i)))
        (if (> temp max) (set max temp)))
      (return max))
    (define ((pre-concat sp_ type-name _reverse) in count out) (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count (set (array-get out (- count i)) (array-get in i))))
    (define ((pre-concat sp_ type-name _equal) in count value)
      (sp-bool-t value-t* sp-size-t value-t)
      (sp-for-each-index i count (if (not (= value (array-get in i))) (return 0)))
      (return 1))
    (define ((pre-concat sp_ type-name _square) in count) (void value-t* sp-size-t)
      (sp-for-each-index i count (set* (array-get in i) (array-get in i))))
    (define ((pre-concat sp_ type-name _add) in-out count value) (void value-t* sp-size-t value-t)
      (sp-for-each-index i count (set+ (array-get in-out i) value)))
    (define ((pre-concat sp_ type-name _multiply) in-out count value)
      (void value-t* sp-size-t value-t)
      (sp-for-each-index i count (set* (array-get in-out i) value)))
    (define ((pre-concat sp_ type-name _divide) in-out count value)
      (void value-t* sp-size-t value-t)
      (sp-for-each-index i count (set/ (array-get in-out i) value)))
    (define ((pre-concat sp_ type-name _set) in-out count value) (void value-t* sp-size-t value-t)
      (sp-for-each-index i count (set (array-get in-out i) value)))
    (define ((pre-concat sp_ type-name _subtract) in-out count value)
      (void value-t* sp-size-t value-t)
      (sp-for-each-index i count (set (array-get in-out i) (subtract (array-get in-out i) value))))
    (define ((pre-concat sp_ type-name _new) count out) (status-t sp-size-t value-t**)
      (return (sph-helper-calloc (* count (sizeof value-t)) out)))
    (define ((pre-concat sp_ type-name _copy) in count out) (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count (set (array-get out i) (array-get in i))))
    (define ((pre-concat sp_ type-name _cusum) in count out) (void value-t* value-t value-t*)
      "calculate cumulative sums from the given numbers.
       (a b c ...) -> (a (+ a b) (+ a b c) ...)"
      (define sum value-t (array-get in 0))
      (set (array-get out 0) sum)
      (for ((define i sp-time-t 1) (< i count) (set+ i 1))
        (set+ sum (array-get in i))
        (set (array-get out i) sum)))
    (define ((pre-concat sp_ type-name _swap) in-out index-1 index-2)
      (void sp-time-t* sp-ssize-t sp-ssize-t)
      (define temp sp-time-t (array-get in-out index-1))
      (set (array-get in-out index-1) (array-get in-out index-2) (array-get in-out index-2) temp))
    (define ((pre-concat sp_ type-name _shuffle) in count) (void value-t* sp-size-t)
      (sp-shuffle
        (convert-type (pre-concat sp_ type-name _swap)
          (function-pointer void void* sp-size-t sp-size-t))
        in count))
    (define ((pre-concat sp_ type-name _array-free) in count) (void value-t** sp-size-t)
      "free every element array and the container array"
      (sp-for-each-index i count (free (array-get in i)))
      (free in))
    (define ((pre-concat sp_ type-name _additions) start summand count out)
      (void value-t value-t value-t value-t*)
      "write count cumulative additions with summand from start to out.
       with summand, only the nth additions are written.
       use case: generating harmonic frequency values"
      (for ((define i sp-size-t 0) (< i count) (set+ i 1 start summand))
        (set (array-get out i) start)))
    (define ((pre-concat sp_ type-name _duplicate) a count out)
      (status-t value-t* sp-size-t value-t**)
      status-declare
      (declare temp value-t*)
      (status-require ((pre-concat sp_ type-name _new) count &temp))
      (memcpy temp a (* count (sizeof value-t)))
      (set *out temp)
      (label exit status-return))
    (define ((pre-concat sp_ type-name _range) in start end out)
      (void value-t* sp-size-t sp-size-t value-t*)
      "write into out values from start (inclusively) to end (exclusively)"
      (sp-for-each-index i (- end start) (set (array-get out i) (array-get in (+ start i)))))
    (define ((pre-concat sp_ type-name _and_ type-name) a b count limit out)
      (void value-t* value-t* sp-size-t value-t value-t*)
      "if a[i] and b[i] greater than limit, take b[i] else 0"
      (sp-for-each-index i count
        (if (and (>= (array-get a i) limit) (>= (array-get a i) limit))
          (set (array-get out i) (array-get b i))
          (set (array-get out i) 0))))
    (define ((pre-concat sp_ type-name _or_ type-name) a b count limit out)
      (void value-t* value-t* sp-size-t value-t value-t*)
      "if a[i] < limit and b[i] > limit, take b[i], else a[i]"
      (sp-for-each-index i count
        (if (< (array-get a i) limit)
          (if (< (array-get b i) limit) (set (array-get out i) (array-get a i))
            (set (array-get out i) (array-get b i)))
          (set (array-get out i) (array-get a i)))))
    (define ((pre-concat sp_ type-name _xor_ type-name) a b count limit out)
      (void value-t* value-t* sp-size-t value-t value-t*)
      "if a[i] > limit and b[i] > limit then 0 else take the one greater than limit"
      (sp-for-each-index i count
        (if (and (>= (array-get a i) limit) (>= (array-get b i) limit)) (set (array-get out i) 0)
          (if (< (array-get a i) limit)
            (if (< (array-get b i) limit) (set (array-get out i) (array-get a i))
              (set (array-get out i) (array-get b i)))
            (set (array-get out i) (array-get a i))))))
    (define ((pre-concat sp_ type-name _multiply_ type-name) in-out count in)
      (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count (set* (array-get in-out i) (array-get in i))))
    (define ((pre-concat sp_ type-name _divide_ type-name) in-out count in)
      (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count (set/ (array-get in-out i) (array-get in i))))
    (define ((pre-concat sp_ type-name _add_ type-name) in-out count in)
      (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count
        (set (array-get in-out i) (+ (array-get in-out i) (array-get in i)))))
    (define ((pre-concat sp_ type-name _subtract_ type-name) in-out count in)
      (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count
        (set (array-get in-out i) (subtract (array-get in-out i) (array-get in i)))))
    (define ((pre-concat sp_ type-name _set_ type-name) in-out count in)
      (void value-t* sp-size-t value-t*)
      (memcpy in-out in (* count (sizeof value-t))))
    (define ((pre-concat sp_ type-name _set_ type-name _left) in-out count in)
      (void value-t* sp-size-t value-t*)
      (sp-for-each-index i count (set (array-get in-out (* -1 i)) (array-get in i))))
    (define ((pre-concat sp_ type-name _harmonic) base count out) (void value-t sp-time-t value-t*)
      "f(n) = base * (n+1)"
      (sp-for-each-index i count (set (array-get out i) (* base (+ 1 i)))))
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
    (define ((pre-concat sp_ type-name _linear) base k count out)
      (void value-t value-t value-t value-t*)
      "a(n) = base – k·n"
      (sp-for-each-index i count (set (array-get out i) (- base (* k i)))))
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