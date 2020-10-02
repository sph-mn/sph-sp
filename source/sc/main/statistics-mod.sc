(sc-comment "routines that modify arrays to change statistical values")
(sc-include "./sc-macros")

(define (sp-stat-times-repetition-decrease a size width target)
  (status-t sp-time-t* sp-time-t sp-time-t sp-time-t)
  "decrease repetition of subseqences of size $width to $target or nearest possible.
   $a is modified directly.
   new sequences are created using existing elements (from width 1 sequences)"
  status-declare
  (declare
    counts-size sp-time-t
    counts sp-times-counted-sequences-t*
    i sp-time-t
    j sp-time-t
    jj sp-time-t
    max-counts-size sp-time-t
    max-unique-set sp-time-t
    max-unique sp-time-t
    max-unique-width sp-time-t
    measured sp-time-t
    min-repetition sp-time-t
    count sp-time-t
    seq-new sp-time-t*
    seq-prev sp-time-t*
    seq sp-time-t*
    set-indices sp-time-t*
    set-size sp-time-t
    set sp-time-t*
    center-i sp-time-t
    difference sp-time-t)
  (if (= 1 width) (begin (sc-comment "not implemented") status-return))
  (memreg-init 4)
  (status-require (sp-times-new size &set))
  (memreg-add set)
  (status-require (sp-times-deduplicate a size set &set-size))
  (set
    max-unique-set (sp-set-sequence-max set-size width)
    max-unique-width (sp-stat-unique-max size width)
    max-unique (sp-min max-unique-set max-unique-width)
    min-repetition (- max-unique-width max-unique)
    target (sp-max min-repetition target)
    max-counts-size (+ (/ max-unique 2) 1))
  (status-require (sp-times-new size &seq-new))
  (memreg-add seq-new)
  (status-require (sp-times-new width &set-indices))
  (memreg-add set-indices)
  (status-require
    (sph-helper-malloc (* max-counts-size (sizeof sp-times-counted-sequences-t)) &counts))
  (memreg-add counts)
  (sp-times-shuffle &sp-default-random-state set set-size)
  (label loop
    (status-require (sp-times-counted-sequences a size width 1 counts &counts-size &measured))
    (if (<= measured target) (goto exit))
    (set difference (- measured target))
    (if counts-size
      (quicksort sp-times-counted-sequences-sort-greater sp-times-counted-sequences-sort-swap
        counts 0 (- counts-size 1)))
    (for-i i counts-size
      (set
        seq (struct-get (array-get counts i) sequence)
        count (struct-get (array-get counts i) count)
        center-i (sp-time-random-bounded &sp-default-random-state size))
      (sc-comment "start at a random index and wrap around")
      (for ((set jj 0) (< jj size) (set+ jj 1))
        (set j (modulo (+ center-i jj) size))
        (if (= 0 (memcmp (+ a j) seq (* width (sizeof sp-time-t))))
          (begin
            (sp-times-select set set-indices width seq-new)
            (sp-times-sequence-increment set-indices width set-size)
            (memcpy (+ a j) seq-new (* width (sizeof sp-time-t)))
            (if (= 1 count) continue (set- count 1))
            (if (= 1 difference) (goto loop) (set- difference 1))))))
    (goto loop))
  (label exit memreg-free status-return))