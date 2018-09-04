(pre-define (define-sp-sine! scm-id id)
  (begin
    "defines scm-sp-sine!, scm-sp-sine-lq!"
    (define (scm-id data len sample-duration freq phase amp) (SCM SCM SCM SCM SCM SCM SCM)
      (id
        (convert-type (SCM-BYTEVECTOR-CONTENTS data) sp-sample-t*)
        (scm->uint32 len)
        (scm->double sample-duration) (scm->double freq) (scm->double phase) (scm->double amp))
      (return SCM-UNSPECIFIED))))

(define-sp-sine! scm-sp-sine! sp-sine)
(define-sp-sine! scm-sp-sine-lq! sp-sine)

(define (init-sp-generate) void
  scm-c-define-procedure-c-init
  (scm-c-define-procedure-c
    "sp-sine!"
    6
    0
    0
    scm-sp-sine!
    "data len sample-duration freq phase amp -> unspecified
    f32vector integer integer rational rational rational rational")
  (scm-c-define-procedure-c
    "sp-sine-lq!"
    6
    0
    0
    scm-sp-sine-lq!
    "data len sample-duration freq phase amp -> unspecified
    f32vector integer integer rational rational rational rational
    faster, lower precision version of sp-sine!.
    currently faster by a factor of about 2.6"))