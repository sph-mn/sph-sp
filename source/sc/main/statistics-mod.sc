(sc-comment "routines that modify arrays to change statistical values")
(sc-include "./sc-macros")

(define (sp-stat-times-repetition-increase a size width target)
  (status-t sp-time-t* sp-time-t sp-time-t sp-time-t)
  "try to increase repetition of subseqences of size $width to $target or nearest possible.
   $a is modified directly.
   uncommon sequences are replaced by more common ones"
  status-declare
  (declare
    a-copy sp-time-t*
    count sp-time-t
    counts-size sp-time-t
    counts sp-times-counted-sequences-t*
    difference sp-time-t
    i sp-time-t
    j sp-time-t
    known sp-sequence-hashtable-t
    loop-limit sp-time-t
    max-unique sp-time-t
    measured sp-time-t
    min-repetition sp-time-t
    seq-a sp-time-t*
    seq-b sp-time-t*
    seq sp-time-t*)
  (memreg-init 2)
  (set
    loop-limit (* size 10)
    max-unique (sp-stat-unique-max size width)
    target (sp-min target (sp-stat-repetition-max size width))
    known.size 0)
  (status-require (sph-helper-malloc (* max-unique (sizeof sp-times-counted-sequences-t)) &counts))
  (memreg-add counts)
  (status-require (sp-times-new size &a-copy))
  (memreg-add a-copy)
  (label loop
    (if (= 1 loop-limit) (goto exit))
    (set- loop-limit 1)
    (if known.size (sp-sequence-hashtable-clear known)
      (sp-sequence-hashtable-new max-unique &known))
    (memcpy a-copy a (* size (sizeof sp-time-t)))
    (sp-times-counted-sequences-hash a-copy size 2 known)
    (sp-times-counted-sequences known 0 counts &counts-size)
    (set measured (- max-unique counts-size))
    (if (>= measured target) (goto exit))
    (set difference (- target measured))
    (if (< 1 counts-size)
      (quicksort sp-times-counted-sequences-sort-less sp-times-counted-sequences-sort-swap
        counts 0 (- counts-size 1)))
    (for ((set i 1) (< i counts-size) (set+ i 1))
      (set
        seq-a (struct-get (array-get counts (- i 1)) sequence)
        seq-b (struct-get (array-get counts (- counts-size i)) sequence)
        count (struct-get (array-get counts (- i 1)) count))
      (for ((set j 0) (< j size) (set+ j 1))
        (set seq (+ a j))
        (if (= 0 (memcmp seq seq-a (* width (sizeof sp-time-t))))
          (begin
            (memcpy seq seq-b (* width (sizeof sp-time-t)))
            (if (= 1 count) break (set- count 1))
            (if (= 1 difference) (goto loop) (set- difference 1))))))
    (goto loop))
  (label exit memreg-free status-return))

(define (sp-stat-times-repetition-decrease a size width target)
  (status-t sp-time-t* sp-time-t sp-time-t sp-time-t)
  "try to decrease repetition of subseqences of size $width to count $target or nearest possible.
   $a is modified directly.
   new sequences are created using existing elements only (width 1 sequences)"
  status-declare
  (declare
    a-copy sp-time-t*
    counts-size sp-time-t
    counts sp-times-counted-sequences-t*
    i sp-time-t
    j sp-time-t
    jj sp-time-t
    known sp-sequence-hashtable-t
    max-unique-set sp-time-t
    max-unique sp-time-t
    max-unique-width sp-time-t
    measured sp-time-t
    min-repetition sp-time-t
    count sp-time-t
    seq-new sp-time-t*
    seq sp-time-t*
    loop-limit sp-time-t
    set-indices sp-time-t*
    set-size sp-time-t
    set sp-time-t*
    key sp-sequence-set-key-t
    value sp-time-t*
    center-i sp-time-t
    difference sp-time-t)
  (if (= 1 width) (begin (sc-comment "not implemented") (goto exit)))
  (memreg-init 5)
  (status-require (sp-times-new size &set))
  (memreg-add set)
  (status-require (sp-times-deduplicate a size set &set-size))
  (set
    max-unique-set (sp-set-sequence-max set-size width)
    max-unique-width (sp-stat-unique-max size width)
    max-unique (sp-min max-unique-set max-unique-width)
    min-repetition (- max-unique-width max-unique)
    target (sp-max min-repetition target)
    known.size 0
    key.size width
    loop-limit (* size 10))
  (status-require (sp-times-new width &seq-new))
  (memreg-add seq-new)
  (set key.data (convert-type seq-new uint8-t*))
  (status-require (sp-times-new width &set-indices))
  (memreg-add set-indices)
  (status-require (sph-helper-malloc (* max-unique (sizeof sp-times-counted-sequences-t)) &counts))
  (memreg-add counts)
  (sp-times-shuffle &sp-default-random-state set set-size)
  (status-require (sp-times-new size &a-copy))
  (memreg-add a-copy)
  (label loop
    (if (= 1 loop-limit) (goto exit))
    (set- loop-limit 1)
    (if known.size (sp-sequence-hashtable-clear known)
      (sp-sequence-hashtable-new max-unique &known))
    (memcpy a-copy a (* size (sizeof sp-time-t)))
    (sp-times-counted-sequences-hash a-copy size width known)
    (sp-times-counted-sequences known 0 counts &counts-size)
    (set measured (- max-unique-width counts-size))
    (if (<= measured target) (goto exit))
    (set difference (- measured target))
    (if (< 1 counts-size)
      (quicksort sp-times-counted-sequences-sort-greater sp-times-counted-sequences-sort-swap
        counts 0 (- counts-size 1)))
    (for-i i counts-size
      (set count (struct-get (array-get counts i) count)) (if (> 2 count) continue)
      (set
        seq (struct-get (array-get counts i) sequence)
        center-i (sp-time-random-bounded &sp-default-random-state size))
      (sc-comment "start at a random index and wrap around to replace more evenly")
      (for ((set jj 0) (< jj size) (set+ jj 1))
        (set j (modulo (+ center-i jj) size))
        (if (= 0 (memcmp (+ a j) seq (* width (sizeof sp-time-t))))
          (begin
            (sc-comment "find a new sequence not included")
            (do-while value
              (sp-times-select set set-indices width seq-new)
              (sp-times-sequence-increment set-indices width set-size)
              (set value (sp-sequence-hashtable-get known key)))
            (memcpy (+ a j) seq-new (* width (sizeof sp-time-t)))
            (if (= 1 count) break (set- count 1))
            (if (= 1 difference) (goto loop) (set- difference 1))))))
    (goto loop))
  (label exit memreg-free (if known.size (sp-sequence-hashtable-free known)) status-return))