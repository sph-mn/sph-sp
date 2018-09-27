(pre-include "libguile.h" "./guile/main.c" "./foreign/guile/sph/guile.c")

(pre-define
  (optional-sample-rate a)
  (if* (scm-is-undefined a) -1
    (scm->int32 a))
  (optional-channel-count a)
  (if* (scm-is-undefined a) -1
    (scm->int32 a))
  (optional-samples a a-len scm)
  (if (scm-is-true scm)
    (set
      a (convert-type (SCM-BYTEVECTOR-CONTENTS scm) sp-sample-t*)
      a-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH scm)))
    (set
      a 0
      a-len 0))
  (optional-index a default)
  (if* (and (not (scm-is-undefined start)) (scm-is-true start)) (scm->uint32 a)
    default)
  (sp-port-type->name a)
  (begin
    "integer -> string"
    (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown")))
  sp-object-type-port 0
  sp-object-type-windowed-sinc 1
  scm-sp-object-type SCM-SMOB-FLAGS
  scm-sp-object-data SCM-SMOB-DATA
  ; error handling
  status-group-db-guile "db-guile"
  (scm-from-status-error a)
  (scm-c-error a.group (db-guile-status-name a) (db-guile-status-description a))
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

(declare
  scm-type-port scm-type-windowed-sinc
  scm-rnrs-raise SCM)

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