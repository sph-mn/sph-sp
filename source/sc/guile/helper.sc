(pre-include "libguile.h" "../main/sph-sp.h" "../foreign/sph/helper.c" "../foreign/sph/guile.c")

(pre-define
  status-group-sp-guile "sp-guile"
  (scm-from-sp-channel-count a) (scm-from-uint32 a)
  (scm-from-sp-sample-count a) (scm-from-uint32 a)
  (scm-from-sp-sample-rate a) (scm-from-uint32 a)
  (scm-from-sp-sample a) (scm-from-double a)
  (scm-from-sp-float a) (scm-from-double a)
  (scm-from-sp-port pointer) (scm-make-foreign-object-1 scm-type-port pointer)
  (scm-from-sp-windowed-sinc pointer) (scm-make-foreign-object-1 scm-type-windowed-sinc pointer)
  (scm->sp-channel-count a) (scm->uint32 a)
  (scm->sp-sample-count a) (scm->uint32 a)
  (scm->sp-sample-rate a) (scm->uint32 a)
  (scm->sp-sample a) (scm->double a)
  (scm->sp-float a) (scm->double a)
  (scm->sp-port a) (convert-type (scm-foreign-object-ref a 0) sp-port-t*)
  (scm->sp-windowed-sinc a) (convert-type (scm-foreign-object-ref a 0) sp-windowed-sinc-state-t*)
  (scm-c-make-sp-samples length)
  (scm-make-f64vector (scm-from-sp-sample-count length) (scm-from-uint8 0))
  (define-sp-sine! scm-id f)
  (begin
    "defines scm-sp-sine!, scm-sp-sine-lq!"
    (define (scm-id scm-data scm-len scm-sample-duration scm-freq scm-phase scm-amp)
      (SCM SCM SCM SCM SCM SCM SCM)
      (f
        (scm->sp-sample-count scm-len)
        (scm->sp-float scm-sample-duration)
        (scm->sp-float scm-freq)
        (scm->sp-float scm-phase)
        (scm->sp-float scm-amp) (convert-type (SCM-BYTEVECTOR-CONTENTS scm-data) sp-sample-t*))
      (return SCM-UNSPECIFIED)))
  ; error handling
  (scm-from-status-error a)
  (scm-c-error a.group (sp-guile-status-name a) (sp-guile-status-description a))
  (scm-c-error group name description)
  (scm-call-1
    scm-rnrs-raise
    (scm-list-4
      (scm-from-latin1-symbol group)
      (scm-from-latin1-symbol name)
      (scm-cons (scm-from-latin1-symbol "description") (scm-from-utf8-string description))
      (scm-cons (scm-from-latin1-symbol "c-routine") (scm-from-latin1-symbol __FUNCTION__))))
  (scm-from-status-return result)
  (return
    (if* status-is-success result
      (scm-from-status-error status)))
  (scm-from-status-dynwind-end-return result)
  (if status-is-success
    (begin
      (scm-dynwind-end)
      (return result))
    (return (scm-from-status-error status))))

(define (sp-guile-status-description a) (uint8-t* status-t)
  "get the description if available for a status"
  (declare b char*)
  (cond
    ((not (strcmp status-group-sp-guile a.group)) (set b ""))
    (else (set b (sp-status-description a))))
  (return (convert-type b uint8-t*)))

(define (sp-guile-status-name a) (uint8-t* status-t)
  "get the name if available for a status"
  (declare b char*)
  (cond
    ((not (strcmp status-group-sp-guile a.group)) (set b "unknown"))
    (else (set b (sp-status-name a))))
  (return (convert-type b uint8-t*)))

(declare
  scm-type-port SCM
  scm-type-windowed-sinc SCM
  scm-rnrs-raise SCM)

#;(
(pre-define
  (optional-sample-rate a)
  (if* (scm-is-undefined a) -1
    (scm->int32 a))
  (optional-channel-count a)
  (if* (scm-is-undefined a) -1
    (scm->int32 a))


  )

(define (scm-take-channel-data a channel-count sample-count) (SCM sp-sample-t** uint32-t uint32-t)
  "get a guile scheme object for channel data sample arrays. returns a list of f64vectors.
  eventually frees given data arrays"
  (declare result SCM)
  (set result SCM-EOL)
  (while channel-count
    (dec channel-count)
    (set result (scm-cons (scm-take-f64vector (array-get a channel-count) sample-count) result)))
  (free a)
  (return result))

(define (scm->channel-data a channel-count sample-count) (sp-sample-t** SCM uint32-t* size-t*)
  "only the result array is allocated, data is referenced to the scm vectors"
  (set *channel-count (scm->uint32 (scm-length a)))
  (if (not *channel-count) (return 0))
  (declare
    result sp-sample-t**
    index size-t)
  (set result (malloc (* *channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0))
  (set
    *sample-count (sp-octets->samples (SCM-BYTEVECTOR-LENGTH (scm-first a)))
    index 0)
  (while (not (scm-is-null a))
    (set (array-get result index)
      (convert-type (SCM-BYTEVECTOR-CONTENTS (scm-first a)) sp-sample-t*))
    (inc index)
    (set a (scm-tail a)))
  (return result))

(define (scm-sp-object-create pointer sp-object-type) (SCM void* uint8-t)
  "sp-object type for storing arbitrary pointers"
  ; failed without local variable and gcc optimisation level 3
  (declare result SCM)
  (set result (scm-new-smob scm-type-sp-object (convert-type pointer scm-t-bits)))
  (SCM-SET-SMOB-FLAGS result (convert-type sp-object-type scm-t-bits))
  (return result))

(define (scm-sp-object-print a output-port print-state) (int SCM SCM scm-print-state*)
  (declare
    result char*
    type uint8-t
    sp-port sp-port-t*)
  (set result (malloc (+ 70 10 7 10 10 2 2)))
  (if (not result) (return 0))
  (set type (scm-sp-object-type a))
  (case = type
    (sp-object-type-port
      (set sp-port (convert-type (scm-sp-object-data a) sp-port-t*))
      (sprintf
        result
        "#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s input?:%s>"
        (convert-type a void*)
        (sp-port-type->name sp-port:type)
        sp-port:sample-rate
        sp-port:channel-count
        (if* (bit-and sp-port-bit-closed sp-port:flags) "#t"
          "#f")
        (if* (bit-and sp-port-bit-input sp-port:flags) "#t"
          "#f")))
    (sp-object-type-windowed-sinc
      (sprintf result "#<sp-state %lx type:windowed-sinc>" (convert-type a void*)))
    (else (sprintf result "#<sp %lx>" (convert-type a void*))))
  ; scm-take eventually frees
  (scm-display (scm-take-locale-string result) output-port)
  (return 0))

(define (scm-sp-object-free a) (size-t SCM)
  (declare
    type uint8-t
    data void*)
  (set
    type (SCM-SMOB-FLAGS a)
    data (convert-type (scm-sp-object-data a) void*))
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


)