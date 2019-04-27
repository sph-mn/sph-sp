(sc-comment
  "quicksort implementation based on the public domain implementation from http://alienryderflex.com/quicksort/")

(pre-define
  quicksort-t int
  quicksort-count-t int
  quicksort-max-levels 1000)

(define (quicksort a a-len) (uint8-t quicksort-t quicksort-count-t)
  (declare
    piv quicksort-t
    beg (array quicksort-t quicksort-max-levels)
    end (array quicksort-t quicksort-max-levels)
    i quicksort-t
    l quicksort-t
    r quicksort-t)
  (set
    i 0
    (array-get beg 0) 0
    (array-get end 0) a-len)
  (while (>= i 0)
    (set
      l (array-get beg i)
      r (- (array-get end i) 1))
    (if (>= l r)
      (begin
        (set i (- i 1))
        continue))
    (set piv (array-get a l))
    (if (= i (- quicksort-max-levels 1)) (return 1))
    (while (< l r)
      (while (and (>= (array-get a r) piv) (< l r))
        (set r (- r 1))
        (if (< l r)
          (set
            l (+ 1 l)
            (array-get a l) (array-get a r))))
      (while (and (<= (array-get a l) piv) (< l r))
        (set l (+ 1 l))
        (if (< l r)
          (set
            r (- r 1)
            (array-get a r) (array-get a l)))))
    (set
      (array-get a l) piv
      (array-get beg (+ 1 i)) (+ 1 l)
      (array-get end (+ 1 i)) (array-get end i)
      i (+ 1 i)
      (array-get end i) l))
  (return 0))