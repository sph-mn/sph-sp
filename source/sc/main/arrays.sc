(sc-comment
  "there are currently two types of arrays:
   * plain memory pointers, called sample-array and time-array
   * structs {.data, .size. .used}, called sp-samples-t and sp-times-t
     * \"size\" is to not have to pass size as an extra function argument
     * \"used\" is an index for variable length content to avoid realloc if the size changes with modifications")

(define (sp-sample-array-new size out) (status-t sp-time-t sp-sample-t**)
  (return (sph-helper-calloc (* size (sizeof sp-sample-t)) out)))

(define (sp-time-array-new size out) (status-t sp-time-t sp-time-t**)
  (return (sph-helper-calloc (* size (sizeof sp-time-t)) out)))

(define (sp-sample-array->time-array in in-size out) (void sp-sample-t* sp-time-t sp-time-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i in-size) (set i (+ 1 i)))
    (set (array-get out i) (sp-cheap-round-positive (array-get in i)))))

(define (sp-sample-array-absolute-max in in-size) (sp-sample-t sp-sample-t* sp-time-t)
  "get the maximum value in samples array, disregarding sign"
  (declare result sp-sample-t a sp-sample-t i sp-time-t)
  (for ((set i 0 result 0) (< i in-size) (set i (+ 1 i)))
    (set a (fabs (array-get in i)))
    (if (> a result) (set result a)))
  (return result))

(define (sp-samples->times a b) (void sp-samples-t sp-times-t)
  (declare i size-t)
  (for ((set i 0) (< i a.size) (set i (+ 1 i)))
    (set (array3-get b i) (sp-cheap-round-positive (array3-get a i)))))