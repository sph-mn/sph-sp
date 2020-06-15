(sc-comment
  "there are currently two types of arrays:
   * plain memory pointers, called samples and times")

(define (sp-samples-new size out) (status-t sp-time-t sp-sample-t**)
  (return (sph-helper-calloc (* size (sizeof sp-sample-t)) out)))

(define (sp-times-new size out) (status-t sp-time-t sp-time-t**)
  (return (sph-helper-calloc (* size (sizeof sp-time-t)) out)))

(define (sp-samples->times in size out) (void sp-sample-t* sp-time-t sp-time-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out i) (sp-cheap-round-positive (array-get in i)))))

(define (sp-samples-absolute-max in in-size) (sp-sample-t sp-sample-t* sp-time-t)
  "get the maximum value in samples array, disregarding sign"
  (declare result sp-sample-t a sp-sample-t i sp-time-t)
  (for ((set i 0 result 0) (< i in-size) (set i (+ 1 i)))
    (set a (fabs (array-get in i)))
    (if (> a result) (set result a)))
  (return result))

(define (sp-samples-display a size) (void sp-sample-t* sp-time-t)
  "display a sample array in one line"
  (declare i sp-time-t)
  (printf "%.5f" (array-get a 0))
  (for ((set i 1) (< i size) (set i (+ 1 i))) (printf " %.5f" (array-get a i)))
  (printf "\n"))

(define (sp-times-display a size) (void sp-time-t* sp-time-t)
  "display a time array in one line"
  (declare i sp-time-t)
  (printf "%lu" (array-get a 0))
  (for ((set i 1) (< i size) (set i (+ 1 i))) (printf " %lu" (array-get a i)))
  (printf "\n"))

(define (sp-samples-set-unity-gain in in-size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  "adjust amplitude of out to match the one of in"
  (declare
    i sp-time-t
    in-max sp-sample-t
    out-max sp-sample-t
    difference sp-sample-t
    correction sp-sample-t)
  (set in-max (sp-samples-absolute-max in in-size) out-max (sp-samples-absolute-max out in-size))
  (if (or (= 0 in-max) (= 0 out-max)) return)
  (set difference (/ out-max in-max) correction (+ 1 (/ (- 1 difference) difference)))
  (for ((set i 0) (< i in-size) (set i (+ 1 i)))
    (set (array-get out i) (* correction (array-get out i)))))