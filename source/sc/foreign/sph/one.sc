;(sc-include-once sph "sph")

(pre-include-once string-h "string.h"
  ; malloc
  stdlib-h "stdlib.h"
  ; "access"
  unistd-h "unistd.h"
  ; mkdir
  sys-stat-h "sys/stat.h"
  ; dirname
  libgen-h "libgen.h" errno-h "errno.h" float-h "float.h")

(pre-define (file-exists? path) (not (equal? (access path F-OK) -1)))
(pre-define (pointer-equal? a b) (= (convert-type a b0*) (convert-type b b0*)))

(define (ensure-trailing-slash a result) (b8 b8* b8**)
  "set result to a new string with a trailing slash added, or the given string if it already has a trailing slash.
  returns 0 if result is the given string, 1 if new memory could not be allocated, 2 if result is a new string"
  (define a-len b32 (strlen a))
  (if (or (not a-len) (equal? #\/ (deref (+ a (- a-len 1)))))
    (begin (set (deref result) a) (return 0))
    (begin (define new-a char* (malloc (+ 2 a-len))) (if (not new-a) (return 1))
      (memcpy new-a a a-len) (memcpy (+ new-a a-len) "/" 1)
      (set (deref new-a (+ 1 a-len)) 0) (set (deref result) new-a) (return 2))))

(define (string-clone a) (b8* b8*)
  "return a new string with the same contents as the given string. return 0 if the memory allocation failed"
  (define a-size size-t (+ 1 (strlen a))) (define result b8* (malloc a-size))
  (if result (memcpy result a a-size)) (return result))

(define (dirname-2 a) (b8* b8*)
  "like posix dirname, but never modifies its argument and always returns a new string"
  (define path-copy b8* (string-clone a)) (return (dirname path-copy)))

(define (ensure-directory-structure path mkdir-mode) (boolean b8* mode-t)
  "return 1 if the path exists or has been successfully created"
  (if (file-exists? path) (return #t)
    (begin (define path-dirname b8* (dirname-2 path))
      (define status boolean (ensure-directory-structure path-dirname mkdir-mode))
      (free path-dirname) (return (and status (or (= EEXIST errno) (= 0 (mkdir path mkdir-mode))))))))

(define (string-append a b) (b8* b8* b8*)
  "always returns a new string" (define a-length size-t (strlen a))
  (define b-length size-t (strlen b)) (define result b8* (malloc (+ 1 a-length b-length)))
  (if result (begin (memcpy result a a-length) (memcpy (+ result a-length) b (+ 1 b-length))))
  (return result))

(define (float-sum numbers len) (f32-s f32-s* b32)
  "sum numbers with rounding error compensation using kahan summation with neumaier modification"
  (define temp f32-s element f32-s) (define correction f32-s 0)
  (set len (- len 1)) (define result f32-s (deref numbers len))
  (while len (set len (- len 1))
    (set element (deref numbers len)) (set temp (+ result element))
    (set correction
      (+ correction
        (if* (>= result element) (+ (- result temp) element) (+ (- element temp) result)))
      result temp))
  (return (+ correction result)))

(define (float-nearly-equal? a b margin) (boolean f32-s f32-s f32-s)
  "approximate float comparison. margin is a factor and is low for low accepted differences.
   http://floating-point-gui.de/errors/comparison/"
  (if (= a b) (return #t)
    (begin (define diff f32-s (fabs (- a b)))
      (return
        (if* (or (= 0 a) (= 0 b) (< diff DBL_MIN)) (< diff (* margin DBL_MIN))
          (< (/ diff (fmin (+ (fabs a) (fabs b)) DBL_MAX)) margin))))))
