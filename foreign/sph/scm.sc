(pre-define scm-first SCM_CAR scm-tail SCM_CDR)

(pre-define
  (scm-c-define-procedure-c scm-temp name required optional rest c-function documentation)
  ;defines and registers/exports a c procedure as scheme procedure with documentation string
  (set scm-temp (scm-c-define-gsubr name required optional rest c-function))
  (scm-set-procedure-property! scm-temp (scm-from-locale-symbol "documentation")
    (scm-from-locale-string documentation)))

(pre-define (scm-c-list-each list e body)
  ;SCM SCM c-compound-expression ->
  (while (not (scm-is-null list)) (set e (scm-first list)) body (set list (scm-tail list))))

(pre-define (scm-is-undefined a) (= SCM-UNDEFINED a))
(pre-define (scm-false-if-undefined a) (if* (scm-is-undefined a) SCM-BOOL-F a))
(pre-define (null-if-undefined a) (if* (scm-is-undefined a) 0 a))
(pre-define (scm-if-undefined-expr a b c) (if* (scm-is-undefined a) b c))
(pre-define (scm-if-undefined a b c) (if (scm-is-undefined a) b c))

(pre-define (scm-is-list-false-or-undefined a)
  (or (scm-is-true (scm-list? a)) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(pre-define (scm-is-integer-false-or-undefined a)
  (or (scm-is-integer a) (= SCM-BOOL-F a) (= SCM-UNDEFINED a)))

(define (scm-c-bytevector-take size-octets a) (SCM size-t b8*)
  ;creates a new bytevector of size-octects from a given bytevector
  (define r SCM (scm-c-make-bytevector size-octets))
  (memcpy (SCM-BYTEVECTOR-CONTENTS r) a size-octets) (return r))

(pre-define (scm-c-error-create name data)
  (scm-call-3 scm-error-create (scm-from-locale-symbol __func__) (if* data data SCM-BOOL-F)))

;scm-c-local-error is the scheme version of sph.c:local-error. it can create an error object defined in the scheme module (sph)

(define scm-error-create SCM
  scm-error? SCM scm-error-origin SCM scm-error-name SCM scm-error-data SCM)

(define (init-scm) b0
  ;the features defined in this file need run-time initialisation. call this once in an application before using the features here
  (define m SCM (scm-c-resolve-module "sph error"))
  (set scm-error-create (scm-variable-ref (scm-c-module-lookup m "error-create-p")))
  (set scm-error-origin (scm-variable-ref (scm-c-module-lookup m "error-origin")))
  (set scm-error-name (scm-variable-ref (scm-c-module-lookup m "error-name")))
  (set scm-error-data (scm-variable-ref (scm-c-module-lookup m "error-data")))
  (set scm-error? (scm-variable-ref (scm-c-module-lookup m "error?"))))

(pre-define scm-c-local-error-init
  (define local-error-origin SCM local-error-name SCM local-error-data SCM))

(pre-define (scm-c-local-error i d)
  (set local-error-origin (scm-from-locale-symbol __func__)
    local-error-name (scm-from-locale-symbol i) local-error-data d)
  (goto error))

(pre-define scm-c-local-error-create
  (scm-call-3 scm-error-create local-error-origin
    local-error-name (if* local-error-data local-error-data SCM-BOOL-F)))

(pre-define (scm-c-local-define-malloc variable-name type)
  (define variable-name type* (malloc (sizeof type)))
  (if (not variable-name) (scm-c-local-error "memory" 0)))

(pre-define (scm-c-local-define-malloc+size variable-name type size)
  (define variable-name type* (malloc size)) (if (not variable-name) (scm-c-local-error "memory" 0)))

(pre-define scm-c-local-error-return (return scm-c-local-error-create))

(pre-if local-error-assert-enable
  (pre-define (scm-c-local-error-assert name expr) (if (not expr) (scm-c-local-error name 0)))
  (pre-define (scm-c-local-error-assert name expr) null))

(pre-define (scm-c-local-error-glibc error-number)
  (scm-c-local-error "glibc" (scm-from-locale-string (strerror error-number))))

(pre-define scm-c-local-error-system
  (scm-c-local-error "system" (scm-from-locale-string (strerror errno))))

(pre-define (scm-c-require-success-glibc a) (set s a) (if (< s 0) (scm-c-local-error-glibc s)))
(pre-define (scm-c-require-success-system a) (if (< a 0) scm-c-local-error-system))
