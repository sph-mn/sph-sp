(pre-include "libguile.h" "generic/main.c" "guile/foreign/sph/guile.c")
(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(pre-define (scm-c-error name description)
  (begin
    "raise an exception with error information set as an alist with given values"
    (scm-call-1
      scm-rnrs-raise
      (scm-list-3
        (scm-from-latin1-symbol name)
        (scm-cons (scm-from-latin1-symbol "description") (scm-from-utf8-string description))
        (scm-cons (scm-from-latin1-symbol "c-routine") (scm-from-latin1-symbol __FUNCTION__))))))

(pre-define (status->scm-error a) (scm-c-error (sp-status-name a) (sp-status-description a)))

(pre-define (status->scm result)
  (if* status-is-success result
    (status->scm-error status)))

(pre-define (status->scm-return result)
  (begin
    "call scm-c-error if status is not success or return result"
    (return (status->scm result))))

(pre-define scm-sp-object-type-init
  (begin
    "sp-object is a guile smob that stores a sub type id and pointer"
    (set scm-type-sp-object (scm-make-smob-type "sp-object" 0))
    (scm-set-smob-print scm-type-sp-object scm-sp-object-print)
    (scm-set-smob-free scm-type-sp-object scm-sp-object-free)))

(pre-define (sp-port-type->name a)
  (begin
    "integer -> string"
    (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown"))))

(pre-define
  sp-object-type-port 0
  sp-object-type-windowed-sinc 1
  scm-sp-object-type SCM-SMOB-FLAGS
  scm-sp-object-data SCM-SMOB-DATA)

(pre-define (optional-sample-rate a)
  (if* (scm-is-undefined a) -1
    (scm->int32 a)))

(pre-define (optional-channel-count a)
  (if* (scm-is-undefined a) -1
    (scm->int32 a)))

(declare
  scm-type-sp-object scm-t-bits
  scm-rnrs-raise SCM)

(define (scm-take-channel-data a channel-count sample-count) (SCM sp-sample-t** b32 b32)
  "get a guile scheme object for channel data sample arrays. returns a list of f64vectors.
  eventually frees given data arrays"
  (define result SCM SCM-EOL)
  (while channel-count
    (dec channel-count)
    (set result (scm-cons (scm-take-f64vector (array-get a channel-count) sample-count) result)))
  (free a)
  (return result))

(define (scm->channel-data a channel-count sample-count) (sp-sample-t** SCM b32* size-t*)
  "only the result array is allocated, data is referenced to the scm vectors"
  (set *channel-count (scm->uint32 (scm-length a)))
  (if (not *channel-count) (return 0))
  (define result sp-sample-t** (malloc (* *channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0))
  (set *sample-count (sp-octets->samples (SCM-BYTEVECTOR-LENGTH (scm-first a))))
  (define index size-t 0)
  (while (not (scm-is-null a))
    (set (array-get result index)
      (convert-type (SCM-BYTEVECTOR-CONTENTS (scm-first a)) sp-sample-t*))
    (inc index)
    (set a (scm-tail a)))
  (return result))

(define (scm-sp-object-create pointer sp-object-type) (SCM b0* b8)
  "sp-object type for storing arbitrary pointers"
  ; failed without local variable and gcc optimisation level 3
  (define result SCM (scm-new-smob scm-type-sp-object (convert-type pointer scm-t-bits)))
  (SCM-SET-SMOB-FLAGS result (convert-type sp-object-type scm-t-bits))
  (return result))

(define (scm-sp-object-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define result char* (malloc (+ 70 10 7 10 10 2 2)))
  (if (not result) (return 0))
  (define type b8 (scm-sp-object-type a))
  (case = type
    (sp-object-type-port
      (define sp-port sp-port-t* (convert-type (scm-sp-object-data a) sp-port-t*))
      (sprintf
        result
        "#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s input?:%s>"
        (convert-type a b0*)
        (sp-port-type->name sp-port:type)
        sp-port:sample-rate
        sp-port:channel-count
        (if* (bit-and sp-port-bit-closed sp-port:flags) "#t"
          "#f")
        (if* (bit-and sp-port-bit-input sp-port:flags) "#t"
          "#f")))
    (sp-object-type-windowed-sinc
      (sprintf result "#<sp-state %lx type:windowed-sinc>" (convert-type a b0*)))
    (else (sprintf result "#<sp %lx>" (convert-type a b0*))))
  ; scm-take eventually frees
  (scm-display (scm-take-locale-string result) output-port)
  (return 0))

(define (scm-sp-object-free a) (size-t SCM)
  (define type b8 (SCM-SMOB-FLAGS a))
  (define data b0* (convert-type (scm-sp-object-data a) b0*))
  (case = type
    (sp-object-type-windowed-sinc (sp-windowed-sinc-state-destroy data))
    (sp-object-type-port (sp-port-close data)))
  (return 0))

(define (scm-float-nearly-equal? a b margin) (SCM SCM SCM SCM)
  (return (scm-from-bool (f64-nearly-equal? (scm->double a) (scm->double b) (scm->double margin)))))

(define (scm-f64vector-sum a start end) (SCM SCM SCM SCM)
  (return
    (scm-from-double
      (f64-sum
        (+
          (if* (scm-is-undefined start) 0
            (scm->size-t start))
          (convert-type (SCM-BYTEVECTOR-CONTENTS a) f64-s*))
        (*
          (if* (scm-is-undefined end) (SCM-BYTEVECTOR-LENGTH a)
            (- end (+ 1 start)))
          (sizeof f64-s))))))

(define (scm-f32vector-sum a start end) (SCM SCM SCM SCM)
  (return
    (scm-from-double
      (f32-sum
        (+
          (if* (scm-is-undefined start) 0
            (scm->size-t start))
          (convert-type (SCM-BYTEVECTOR-CONTENTS a) f32-s*))
        (*
          (if* (scm-is-undefined end) (SCM-BYTEVECTOR-LENGTH a)
            (- end (+ 1 start)))
          (sizeof f32-s))))))

(pre-include "guile/io.c" "guile/generate.c" "guile/transform.c")

(define (sp-init-guile) b0
  (init-sp-io)
  (init-sp-generate)
  (init-sp-transform)
  scm-c-define-procedure-c-init
  scm-sp-object-type-init
  (set scm-rnrs-raise (scm-c-public-ref "rnrs exceptions" "raise"))
  (scm-c-define-procedure-c
    "f32vector-sum" 1 2 0 scm-f32vector-sum "f32vector [start end] -> number")
  (scm-c-define-procedure-c
    "f64vector-sum" 1 2 0 scm-f64vector-sum "f64vector [start end] -> number")
  (scm-c-define-procedure-c
    "float-nearly-equal?"
    3 0 0 scm-float-nearly-equal?
    "a b margin -> boolean
    number number number -> boolean"))