(define-macro scm-first SCM_CAR scm-tail SCM_CDR)

(define-macro
  (scm-c-define-procedure-c scm-temp name required optional rest c-function documentation)
  ;defines and registers/exports a c procedure as scheme procedure with documentation string
  (set scm-temp (scm-c-define-gsubr name required optional rest c-function))
  (scm-set-procedure-property! scm-temp (scm-from-locale-symbol "documentation")
    (scm-from-locale-string documentation)))

(define-macro (scm-c-list-each list e body)
  ;SCM SCM c-compound-expression ->
  (while (not (scm-is-null list)) (set e (scm-first list)) body (set list (scm-tail list))))

(define-macro (false-if-undefined a) (if* (= SCM-UNDEFINED a) SCM-BOOL-F a))
(define-macro (null-if-undefined a) (if* (= SCM-UNDEFINED a) 0 a))
(define-macro (scm-if-undefined-expr a b c) (if* (= SCM-UNDEFINED a) b c))
(define-macro (scm-if-undefined a b c) (if (= SCM-UNDEFINED a) b c))

(define-macro (scm-is-list-false-or-undefined a)
  (or (scm-is-true (scm-list? a)) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define-macro (scm-is-integer-false-or-undefined a)
  (or (scm-is-integer a) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define (scm-c-bytevector-take size-octets a) (SCM size-t b8*)
  ;creates a new bytevector of size-octects from a given bytevector
  (define r SCM (scm-c-make-bytevector size-octets))
  (memcpy (SCM-BYTEVECTOR-CONTENTS r) a size-octets) (return r))

;scm-c-local-error is the scheme version of sph.c:local-error. it can create an error object defined in the scheme module (sph)

(define-macro scm-c-local-error-init
  (define local-error-origin SCM local-error-name SCM local-error-data SCM))

(define-macro (scm-c-local-error i d)
  (set local-error-origin (scm-from-locale-symbol __func__)
    local-error-name (scm-from-locale-symbol i) local-error-data d)
  (goto error))

(define-macro (scm-c-local-error-create)
  (scm-call-3 scm-make-error local-error-origin
    local-error-name (if* local-error-data local-error-data SCM-BOOL-F)))

(define-macro (scm-c-local-define-malloc variable-name type)
  (define variable-name type* (malloc (sizeof type)))
  (if (not variable-name) (scm-c-local-error "memory" 0)))

(define-macro (scm-c-local-define-malloc+size variable-name type size)
  (define variable-name type* (malloc size)) (if (not variable-name) (scm-c-local-error "memory" 0)))

(define-macro (scm-c-local-error-return) (return (scm-c-local-error-create)))

(pre-if local-error-assert-enable
  (define-macro (scm-c-local-error-assert name expr) (if (not expr) (scm-c-local-error name 0)))
  (define-macro (scm-c-local-error-assert name expr) null))

(define scm-make-error SCM
  scm-error? SCM scm-error-origin SCM scm-error-name SCM scm-error-data SCM)

(define (init-scm) b0
  ;the features defined in this file need run-time initialisation. call this once in an application before using the features here
  (define m SCM (scm-c-resolve-module "sph"))
  (set scm-make-error (scm-variable-ref (scm-c-module-lookup m "make-error-p")))
  (set scm-error-origin (scm-variable-ref (scm-c-module-lookup m "error-origin-p")))
  (set scm-error-name (scm-variable-ref (scm-c-module-lookup m "error-name-p")))
  (set scm-error-data (scm-variable-ref (scm-c-module-lookup m "error-data-p")))
  (set scm-error? (scm-variable-ref (scm-c-module-lookup m "error?-p"))))

(define-macro (scm-c-local-error-glibc error-number)
  (scm-c-local-error "glibc" (scm-from-locale-string (strerror error-number))))

(define-macro (scm-c-require-success-glibc a) (set s a) (if (< s 0) (scm-c-local-error-glibc s)))