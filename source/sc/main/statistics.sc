(sc-comment
  "calculate statistics for arrays, with the statistics to calculate being selected by an array of identifiers.
   deviation: standard deviation
   complexity: subsequence width with the highest proportion of equal-size unique subsequences")

(sc-define-syntax (for-i index limit body ...)
  (for ((set index 0) (< index limit) (set+ index 1)) body ...))

(sc-define-syntax (define-sp-stat-times name body ...)
  (define (name a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*) body ...))

(sc-define-syntax (define-sp-stat-samples name body ...)
  (define (name a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*) body ...))

(sc-define-syntax (define-sp-stat2-times name body ...)
  (define (name a b size out) (uint8-t sp-time-t* sp-time-t* sp-time-t sp-sample-t*) body ...))

(sc-define-syntax (define-sp-stat2-samples name body ...)
  (define (name a b size out) (uint8-t sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*) body ...))

(pre-define
  (define-sp-stat-range name value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "out: min, max, range"
    (declare i sp-time-t min value-t max value-t b value-t)
    (set min (array-get a 0) max min)
    (for-i i size (set b (array-get a i)) (if (> b max) (set max b) (if (< b min) (set min b))))
    (set (array-get out 0) min (array-get out 1) max (array-get out 2) (- max min))
    (return 0))
  (define-sp-stat-deviation name stat-mean value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    (declare i sp-time-t sum sp-sample-t dev sp-sample-t mean sp-sample-t)
    (stat-mean a size &mean)
    (set sum 0)
    (for-i i size (set dev (- (array-get a i) mean) sum (+ sum (* dev dev))))
    (set *out (sqrt (/ sum size)))
    (return 0))
  (define-sp-stat-skewness name stat-mean value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "mean((x - mean(data)) ** 3) / (mean((x - mean(data)) ** 2) ** 3/2)"
    (declare i sp-time-t mean sp-sample-t m3 sp-sample-t m2 sp-sample-t b sp-sample-t)
    (set m2 0 m3 0)
    (stat-mean a size &mean)
    (for-i i size (set b (- (array-get a i) mean) m2 (+ m2 (* b b)) m3 (+ m3 (* b b b))))
    (set m3 (/ m3 size) m2 (/ m2 size) *out (/ m3 (sqrt (* m2 m2 m2))))
    (return 0))
  (define-sp-stat-kurtosis name stat-mean value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "mean((x - mean(data)) ** 4) / (mean((x - mean(data)) ** 2) ** 2)"
    (declare b sp-sample-t i sp-time-t mean sp-sample-t m2 sp-sample-t m4 sp-sample-t)
    (set m2 0 m4 0)
    (stat-mean a size &mean)
    (for-i i size (set b (- (array-get a i) mean) m2 (+ m2 (* b b)) m4 (+ m4 (* b b b b))))
    (set m4 (/ m4 size) m2 (/ m2 size) *out (/ m4 (* m2 m2)))
    (return 0))
  (define-sp-stat-median name sort-less sort-swap value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    (declare temp value-t*)
    (set temp (malloc (* size (sizeof value-t))))
    (if (not temp) (return 1))
    (memcpy temp a (* size (sizeof value-t)))
    (quicksort sort-less sort-swap temp 0 (- size 1))
    (set *out
      (if* (bit-and size 1) (array-get temp (- (/ size 2) 1))
        (/ (+ (array-get temp (/ size 2)) (array-get temp (- (/ size 2) 1))) 2.0)))
    (return 0))
  (define-sp-stat-inharmonicity name value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "n1: 0..n; n2 = 0..n; half_offset(x) = 0.5 >= x ? x : x - 1;
     min(for(n1, mean(for(n2, half_offset(x(n2) / x(n1))))))"
    (declare i sp-time-t i2 sp-time-t b sp-sample-t sum sp-sample-t min sp-sample-t)
    (set sum 0 min size)
    (for ((set i 0) (< i size) (set+ i 1))
      (set sum 0)
      (for-i i2 size
        (set
          b (/ (array-get a i2) (convert-type (array-get a i) sp-sample-t))
          b (- b (sp-cheap-floor-positive b))
          b (if* (>= 0.5 b) b (- 1 b))
          sum (+ sum b)))
      (set sum (/ sum size))
      (if (< sum min) (set min sum)))
    (set *out min)
    (return 0))
  (define-sp-stat name f-array value-t)
  (define (name a a-size stats size out)
    (status-t value-t* sp-time-t sp-stat-type-t* sp-time-t sp-sample-t*)
    "write to out the statistics requested with sp-stat-type-t indices in stats.
     out size is expected to be at least sp-stat-types-count"
    (declare i sp-time-t)
    status-declare
    (for-i i size
      (status-i-require
        ((array-get f-array (array-get stats i)) a a-size (+ out (array-get stats i)))))
    (label exit status-return)))

(sc-comment "times")

(define-sp-stat-times sp-stat-times-center (declare i sp-time-t sum sp-time-t index-sum sp-time-t)
  (set index-sum 0 sum (array-get a 0))
  (for-i i size (set+ sum (array-get a i) index-sum (* i (array-get a i))))
  (set *out (/ index-sum (convert-type sum sp-sample-t))) (return 0))

(define-sp-stat-range sp-stat-times-range sp-time-t)

(define-sp-stat-times sp-stat-times-complexity
  (declare
    known sequence-set-t
    key sequence-set-key-t
    max-ratio sp-sample-t
    max-ratio-width sp-time-t
    value sequence-set-key-t*
    count sp-time-t
    i sp-time-t
    ratio sp-sample-t)
  (if (sequence-set-new size &known) (return 1)) (set max-ratio 0 max-ratio-width 0)
  (for ((set key.size 1) (< key.size size) (set+ key.size 1))
    (set count 0)
    (for-i i (- size (- key.size 1))
      (set key.data (convert-type (+ i a) uint8-t*) value (sequence-set-get known key))
      (if (not value)
        (begin
          (set+ count 1)
          (if (not (sequence-set-add known key)) (begin (sequence-set-free known) (return 1))))))
    (sc-comment
      "one pattern per width is always unique, exclude because only repetition is counted")
    (set ratio (/ (- count 1) (convert-type (- size (- key.size 1)) sp-sample-t)))
    (if (> ratio max-ratio) (set max-ratio ratio max-ratio-width key.size))
    (sequence-set-clear known))
  (if (not max-ratio-width) (set max-ratio 1 max-ratio-width size))
  (set (array-get out 0) max-ratio (array-get out 1) max-ratio-width) (sequence-set-free known)
  (return 0))

(define-sp-stat-times sp-stat-times-mean (declare i sp-time-t sum sp-time-t)
  (set sum 0) (for-i i size (set+ sum (array-get a i)))
  (set *out (/ sum (convert-type size sp-sample-t))) (return 0))

(define-sp-stat-deviation sp-stat-times-deviation sp-stat-times-mean sp-time-t)
(define-sp-stat-median sp-stat-times-median sp-times-sort-less sp-times-sort-swap sp-time-t)
(define-sp-stat-skewness sp-stat-times-skewness sp-stat-times-mean sp-time-t)
(define-sp-stat-kurtosis sp-stat-times-kurtosis sp-stat-times-mean sp-time-t)
(sc-comment "samples")

(define-sp-stat-samples sp-stat-samples-center
  (declare i sp-time-t sum sp-sample-t index-sum sp-sample-t)
  (set index-sum 0 sum (sp-samples-sum a size)) (for-i i size (set+ index-sum (* i (array-get a i))))
  (set *out (/ index-sum sum)) (return 0))

(define-sp-stat-range sp-stat-samples-range sp-sample-t)
(define-sp-stat-inharmonicity sp-stat-times-inharmonicity sp-time-t)

(define (sp-samples-scale->times a size max out) (void sp-sample-t* sp-time-t sp-time-t sp-time-t*)
  "make all values positive then scale by multiplication so that the largest value is max
   then round to integer"
  (declare i sp-time-t range (array sp-sample-t 3) addition sp-sample-t)
  (sc-comment "returns range, min, max")
  (sp-stat-samples-range a size range)
  (set addition (if* (> 0 (array-get range 1)) (fabs (array-get range 1)) 0))
  (for-i i size
    (set (array-get out i)
      (sp-cheap-round-positive (* (+ (array-get a i) addition) (/ max (array-get range 0)))))))

(define-sp-stat-samples sp-stat-samples-complexity (declare b sp-time-t*)
  status-declare (status-require (sp-times-new size &b)) (sp-samples-scale->times a size 1000 b)
  (sp-stat-times-complexity b size out) (free b) (label exit (return status.id)))

(define-sp-stat-samples sp-stat-samples-mean (set *out (/ (sp-samples-sum a size) size)) (return 0))
(define-sp-stat-deviation sp-stat-samples-deviation sp-stat-samples-mean sp-sample-t)
(define-sp-stat-median sp-stat-samples-median sp-samples-sort-less sp-samples-sort-swap sp-sample-t)
(define-sp-stat-skewness sp-stat-samples-skewness sp-stat-samples-mean sp-sample-t)
(define-sp-stat-kurtosis sp-stat-samples-kurtosis sp-stat-samples-mean sp-sample-t)
(define-sp-stat-inharmonicity sp-stat-samples-inharmonicity sp-sample-t)

(sc-comment
  "f-array maps sp-stat-type-t indices to the functions that calculate the corresponding values")

(define-sp-stat sp-stat-times sp-stat-times-f-array sp-time-t)
(define-sp-stat sp-stat-samples sp-stat-samples-f-array sp-sample-t)