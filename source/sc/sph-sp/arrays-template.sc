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
      (sp-for-each-index i count (set (array-get out i) start) (set+ start summand)))
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
        (set (array-get in-out i) (subtract (array-get in-out i) (array-get in i)))))))