(define (scm-sp-convolve! result a b carryover carryover-len) (SCM SCM SCM SCM SCM SCM)
  (define a-len b32 (sp-octets->samples (SCM-BYTEVECTOR-LENGTH a)))
  (define b-len b32 (sp-octets->samples (SCM-BYTEVECTOR-LENGTH b)))
  (sp-convolve
    (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*)
    a-len
    (convert-type (SCM-BYTEVECTOR-CONTENTS b) sp-sample-t*)
    b-len (convert-type (SCM-BYTEVECTOR-CONTENTS carryover) sp-sample-t*) (scm->size-t carryover-len))
  (return SCM-UNSPECIFIED))

(pre-define (optional-samples a a-len scm)
  (if (scm-is-true scm)
    (set
      a (convert-type (SCM-BYTEVECTOR-CONTENTS scm) sp-sample-t*)
      a-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH scm)))
    (set
      a 0
      a-len 0)))

(pre-define (optional-index a default)
  (if* (and (not (scm-is-undefined start)) (scm-is-true start)) (scm->uint32 a) default))

(define (scm-sp-moving-average! result source scm-prev scm-next distance start end)
  (SCM SCM SCM SCM SCM SCM SCM SCM)
  (define source-len b32 (sp-octets->samples (SCM-BYTEVECTOR-LENGTH source)))
  (define
    prev sp-sample-t*
    prev-len b32
    next sp-sample-t*
    next-len b32)
  (optional-samples prev prev-len scm-prev)
  (optional-samples next next-len scm-next)
  (sp-moving-average
    (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*)
    source-len
    prev
    prev-len
    next
    next-len (optional-index start 0) (optional-index end (- source-len 1)) (scm->uint32 distance))
  (return SCM-UNSPECIFIED))

(define (scm-sp-windowed-sinc-state-create sample-rate freq transition old-state)
  (SCM SCM SCM SCM SCM)
  (define state sp-windowed-sinc-state-t*)
  (if (scm-is-true old-state)
    (set state (convert-type (scm-sp-object-data old-state) sp-windowed-sinc-state-t*)) (set state 0))
  (sp-windowed-sinc-state-create
    (scm->uint32 sample-rate) (scm->double freq) (scm->double transition) (address-of state))
  ; old-state has either been updated or a new state been created
  (return
    (if* (scm-is-true old-state)
      old-state (scm-sp-object-create state sp-object-type-windowed-sinc))))

(define (scm-sp-windowed-sinc! result source state) (SCM SCM SCM SCM)
  ;(define source-len b32 (sp-octets->samples (SCM-BYTEVECTOR-LENGTH source)))
  #;(define
    prev sp-sample-t*
    prev-len b32
    next sp-sample-t*
    next-len b32)
  ;(define state sp-windowed-sinc-state-t* (scm-sp-object-data state))
  #;(sp-windowed-sinc (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) source-len
    (optional-index start 0) (optional-index end (- source-len 1)) state)
  (return SCM-UNSPECIFIED))

(define (scm-sp-fft source) (SCM SCM)
  status-init
  (define result-len b32 (/ (* 3 (SCM-BYTEVECTOR-LENGTH source)) 2))
  (define result SCM (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require!
    (sp-fft
      (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
      result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit
    (status->scm-return result)))

(define (scm-sp-ifft source) (SCM SCM)
  status-init
  (define result-len b32 (* (- (SCM-BYTEVECTOR-LENGTH source) 1) 2))
  (define result SCM (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require!
    (sp-ifft
      (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
      result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit
    (status->scm-return result)))

(define (init-sp-transform) b0
  scm-c-define-procedure-c-init
  (scm-c-define-procedure-c
    "sp-fft"
    1
    0
    0
    scm-sp-fft
    "f32vector:value-per-time -> f32vector:frequencies-per-time
    discrete fourier transform on the input data")
  (scm-c-define-procedure-c
    "sp-ifft"
    1
    0
    0
    scm-sp-ifft
    "f32vector:frequencies-per-time -> f32vector:value-per-time
    inverse discrete fourier transform on the input data")
  (scm-c-define-procedure-c
    "sp-moving-average!"
    5
    2
    0
    scm-sp-moving-average!
    "result source previous next distance [start end] -> unspecified
  f32vector f32vector f32vector f32vector integer integer integer [integer]")
  (scm-c-define-procedure-c
    "sp-windowed-sinc!"
    7
    2
    0
    scm-sp-windowed-sinc!
    "result source previous next sample-rate freq transition [start end] -> unspecified
    f32vector f32vector f32vector f32vector number number integer integer -> boolean")
  (scm-c-define-procedure-c
    "sp-convolve!" 3 0 0 scm-sp-convolve! "a b state:(integer . f32vector) -> state"))
