(define-macro (scm-c-define-procedure-c scm-temp name required optional rest c-function documentation)
  (set scm-temp (scm-c-define-gsubr name required optional rest c-function))
  (scm-set-procedure-property! scm-temp (scm-from-locale-symbol "documentation")
    (scm-from-locale-string documentation)))

(define-macro scm-first SCM_CAR scm-tail SCM_CDR)

(define-macro (scm-c-list-each list e body)
  ;SCM SCM c-compound-expression
  (while (not (scm-is-null list)) (set e (scm-first list)) body (set list (scm-tail list))))

(define-macro (false-if-undefined a) (if* (= SCM-UNDEFINED a) SCM-BOOL-F a))
(define-macro (null-if-undefined a) (if* (= SCM-UNDEFINED a) 0 a))

(define-macro (scm-is-list-false-or-undefined a)
  (or (scm-is-true (scm-list? a)) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define-macro (scm-is-integer-false-or-undefined a)
  (or (scm-is-integer a) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define (scm-bytevector-from-data a) (SCM b8*)
  (define r SCM (scm-c-make-bytevector size-data)) (memcpy (SCM-BYTEVECTOR-CONTENTS r) a size-data)
  (return r))
