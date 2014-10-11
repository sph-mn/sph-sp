(define-macro scm-first SCM_CAR scm-tail SCM_CDR)

(define-macro
  (scm-c-define-procedure-c scm-temp name required optional rest c-function documentation)
  (set scm-temp (scm-c-define-gsubr name required optional rest c-function))
  (scm-set-procedure-property! scm-temp (scm-from-locale-symbol "documentation")
    (scm-from-locale-string documentation)))

(define-macro (scm-c-list-each list e body)
  ;SCM SCM c-compound-expression
  (while (not (scm-is-null list)) (set e (scm-first list)) body (set list (scm-tail list))))

(define-macro (false-if-undefined a) (if* (= SCM-UNDEFINED a) SCM-BOOL-F a))
(define-macro (null-if-undefined a) (if* (= SCM-UNDEFINED a) 0 a))

(define-macro (scm-is-list-false-or-undefined a)
  (or (scm-is-true (scm-list? a)) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define-macro (scm-is-integer-false-or-undefined a)
  (or (scm-is-integer a) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define (scm-c-bytevector-take size-octets a) (SCM size-t b8*)
  (define r SCM (scm-c-make-bytevector size-octets))
  (memcpy (SCM-BYTEVECTOR-CONTENTS r) a size-octets) (return r))

(define-macro (scm-if-undefined-expr a b c) (if* (= SCM-UNDEFINED a) b c))
(define-macro (scm-if-undefined a b c) (if (= SCM-UNDEFINED a) b c))
(define-macro scm-c-local-error-init (define local-error-name b8* local-error-description b8*))

(define-macro (scm-c-local-error i d)
  (set local-error-name (convert-type i b8*) local-error-description (convert-type d b8*))
  (goto error))

(define-macro (scm-c-local-error-create)
  (scm-call-2 scm-make-error (scm-from-locale-symbol (convert-type local-error-name char*))
    (scm-from-locale-string (convert-type local-error-description char*))))

(define-macro (scm-c-local-define-malloc variable-name type)
  (define variable-name type* (malloc (sizeof type)))
  (if (not variable-name) (scm-c-local-error "memory" "")))

(define-macro (scm-c-local-error-return) (return (scm-c-local-error-create)))

(pre-if local-error-assert-enable
  (define-macro (scm-c-local-error-assert name expr) (if (not expr) (scm-c-local-error name "")))
  (define-macro (scm-c-local-error-assert name expr) null))

(define scm-make-error SCM scm-error? SCM scm-error-name SCM scm-error-data SCM)

(define (init-scm) b0
  (define m SCM (scm-c-resolve-module "sph"))
  (set scm-make-error (scm-variable-ref (scm-c-module-lookup m "make-error")))
  (set scm-error-name (scm-variable-ref (scm-c-module-lookup m "error-name")))
  (set scm-error-data (scm-variable-ref (scm-c-module-lookup m "error-data")))
  (set scm-error? (scm-variable-ref (scm-c-module-lookup m "error?"))))

(define-macro (scm-c-local-error-glibc error-number)
  (scm-c-local-error "glibc" (strerror error-number)))

(define-macro (scm-c-require-success-glibc a) (set s a) (if (< s 0) (scm-c-local-error-glibc s)))