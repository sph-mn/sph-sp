(pre-include "./helper.c")
(define-sp-sine! scm-sp-sine! sp-sine)
(define-sp-sine! scm-sp-sine-lq! sp-sine)

(define (scm-sp-port-channel-count scm-a) (SCM SCM)
  (return (scm-from-sp-channel-count (: (scm->sp-port scm-a) channel-count))))

(define (scm-sp-port-sample-rate scm-a) (SCM SCM)
  (return (scm-from-sp-sample-rate (: (scm->sp-port scm-a) sample-rate))))

(define (scm-sp-port-position? scm-a) (SCM SCM)
  (return (scm-from-bool (bit-and sp-port-bit-position (: (scm->sp-port scm-a) flags)))))

(define (scm-sp-port-input? scm-a) (SCM SCM)
  (return (scm-from-bool (bit-and sp-port-bit-input (: (scm->sp-port scm-a) flags)))))

(define (scm-sp-port-position scm-a) (SCM SCM)
  "returns the current port position offset in number of samples"
  (declare position sp-sample-count-t)
  (sp-port-position (scm->sp-port scm-a) &position)
  (return (scm-from-sp-sample-count position)))

(define (scm-sp-port-close a) (SCM SCM)
  status-declare
  (set status (sp-port-close (scm->sp-port a)))
  (scm-from-status-return SCM-UNSPECIFIED))

(define (scm-sp-port-set-position scm-port scm-sample-offset) (SCM SCM SCM)
  status-declare
  (set status
    (sp-port-set-position (scm->sp-port scm-port) (scm->sp-sample-count scm-sample-offset)))
  (scm-from-status-return SCM-UNSPECIFIED))

(define (scm-sp-convolve! result a b carryover) (SCM SCM SCM SCM SCM)
  (declare
    a-len sp-sample-count-t
    b-len sp-sample-count-t
    c-len sp-sample-count-t)
  (set
    a-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH a))
    b-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH b))
    c-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH b)))
  (if (< c-len b-len)
    (scm-c-error
      status-group-sp-guile
      "invalid-argument-size"
      "carryover argument bytevector must be at least as large as the second argument bytevector"))
  (sp-convolve
    (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*)
    a-len
    (convert-type (SCM-BYTEVECTOR-CONTENTS b) sp-sample-t*)
    b-len
    c-len
    (convert-type (SCM-BYTEVECTOR-CONTENTS carryover) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*))
  (return SCM-UNSPECIFIED))

(define (scm-sp-windowed-sinc-state-create scm-sample-rate scm-freq scm-transition scm-state)
  (SCM SCM SCM SCM SCM)
  status-declare
  (declare state sp-windowed-sinc-state-t*)
  (set state
    (if* (and (not SCM-UNDEFINED) (scm-is-true scm-state)) (scm->sp-windowed-sinc scm-state)
      0))
  (status-require
    (sp-windowed-sinc-state-create
      (scm->sp-sample-rate scm-sample-rate)
      (scm->sp-float scm-freq) (scm->sp-float scm-transition) &state))
  (set scm-state (scm-from-sp-windowed-sinc state))
  (label exit
    (scm-from-status-return scm-state)))

(define
  (scm-sp-windowed-sinc! scm-result scm-source scm-sample-rate scm-freq scm-transition scm-state)
  (SCM SCM SCM SCM SCM SCM SCM)
  status-declare
  (declare state sp-windowed-sinc-state-t*)
  (set
    state (scm->sp-windowed-sinc scm-state)
    status
    (sp-windowed-sinc
      (convert-type (SCM-BYTEVECTOR-CONTENTS scm-source) sp-sample-t*)
      (sp-octets->samples (SCM-BYTEVECTOR-LENGTH scm-source))
      (scm->sp-sample-rate scm-sample-rate)
      (scm->sp-float scm-freq)
      (scm->sp-float scm-transition)
      &state (convert-type (SCM-BYTEVECTOR-CONTENTS scm-result) sp-sample-t*)))
  (scm-from-status-return SCM-UNSPECIFIED))

(define
  (scm-sp-moving-average! scm-result scm-source scm-prev scm-next scm-radius scm-start scm-end)
  (SCM SCM SCM SCM SCM SCM SCM SCM)
  status-declare
  (declare
    source-len sp-sample-count-t
    prev-len sp-sample-count-t
    next-len sp-sample-count-t
    prev sp-sample-t*
    next sp-sample-t*
    start sp-sample-count-t
    end sp-sample-count-t)
  (set
    source-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH scm-source))
    start
    (if* (and (not (scm-is-undefined scm-start)) (scm-is-true scm-start))
      (scm->sp-sample-count scm-start)
      0)
    end
    (if* (and (not (scm-is-undefined scm-end)) (scm-is-true scm-end))
      (scm->sp-sample-count scm-end)
      (- source-len 1)))
  (if (scm-is-true scm-prev)
    (set
      prev (convert-type (SCM-BYTEVECTOR-CONTENTS scm-prev) sp-sample-t*)
      prev-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH scm-prev)))
    (set
      prev 0
      prev-len 0))
  (if (scm-is-true scm-next)
    (set
      next (convert-type (SCM-BYTEVECTOR-CONTENTS scm-next) sp-sample-t*)
      next-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH scm-next)))
    (set
      next 0
      next-len 0))
  (status-require
    (sp-moving-average
      (convert-type (SCM-BYTEVECTOR-CONTENTS scm-source) sp-sample-t*)
      source-len
      prev
      prev-len
      next
      next-len
      (scm->sp-sample-count scm-radius)
      start end (convert-type (SCM-BYTEVECTOR-CONTENTS scm-result) sp-sample-t*)))
  (label exit
    (scm-from-status-return SCM-UNSPECIFIED)))

(define (scm-sp-fft scm-source) (SCM SCM)
  status-declare
  (declare
    result-len sp-sample-count-t
    scm-result SCM)
  (set
    result-len (/ (* 3 (SCM-BYTEVECTOR-LENGTH scm-source)) 2)
    scm-result (scm-c-make-sp-samples result-len))
  (status-require
    (sp-fft
      result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS scm-source) sp-sample-t*)
      (SCM-BYTEVECTOR-LENGTH scm-source)
      (convert-type (SCM-BYTEVECTOR-CONTENTS scm-result) sp-sample-t*)))
  (label exit
    (scm-from-status-return scm-result)))

(define (scm-sp-ifft scm-source) (SCM SCM)
  status-declare
  (declare
    result-len sp-sample-count-t
    scm-result SCM)
  (set
    result-len (* (- (SCM-BYTEVECTOR-LENGTH scm-source) 1) 2)
    scm-result (scm-c-make-sp-samples result-len))
  (status-require
    (sp-ifft
      result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS scm-source) sp-sample-t*)
      (SCM-BYTEVECTOR-LENGTH scm-source)
      (convert-type (SCM-BYTEVECTOR-CONTENTS scm-result) sp-sample-t*)))
  (label exit
    (scm-from-status-return scm-result)))

#;(
(define (scm-sp-port-read scm-port scm-sample-count) (SCM SCM SCM)
  status-declare
  (declare
    port sp-port-t*
    sample-count sp-sample-count-t
    channel-count sp-sample-count-t
    data sp-sample-t**
    result SCM)
  (set
    port (scm-sp-port scm-port)
    sample-count (scm->uint32 scm-sample-count)
    channel-count (: port channel-count)
    data (sp-alloc-channel-array channel-count sample-count))
  (sp-status-require-alloc data)
  (status-require (sp-port-read data port sample-count))
  (set result (scm-take-channel-data data channel-count sample-count))
  (label exit
    (status->scm-return result)))

(define (scm-sp-port-write scm-port scm-channel-data scm-sample-count) (SCM SCM SCM SCM)
  status-declare
  (declare
    port sp-port-t*
    data sp-sample-t**
    channel-count sp-sample-count-t
    sample-count size-t)
  (local-memory-init 1)
  (set
    port (scm-sp-port scm-port)
    data (scm->channel-data scm-channel-data &channel-count &sample-count))
  (sp-status-require-alloc data)
  (local-memory-add data)
  (status-require (sp-port-write port sample-count data))
  (label exit
    local-memory-free
    (status->scm-return SCM-UNSPECIFIED)))

(define (scm-sp-file-open scm-path scm-channel-count scm-sample-rate) (SCM SCM SCM SCM)
  status-declare
  (declare
    path uint8-t*
    channel-count sp-sample-count-t
    sample-rate sp-sample-count-t
    result SCM)
  (set
    path (scm->locale-string scm-path)
    channel-count (scm->uint32 scm-channel-count)
    sample-rate (scm->uint32 scm-sample-rate))
  (sp-alloc-define sp-port sp-port-t* (sizeof sp-port-t))
  (status-require (sp-file-open sp-port path channel-count sample-rate))
  (set result (scm-sp-object-create sp-port sp-object-type-port))
  (label exit
    (status->scm-return result)))

(define (scm-sp-alsa-open scm-device-name scm-input? scm-channel-count scm-sample-rate scm-latency)
  (SCM SCM SCM SCM SCM SCM)
  status-declare
  (declare
    device-name uint8-t*
    input? boolean
    channel-count sp-sample-count-t
    sample-rate sp-sample-count-t
    latency sp-sample-count-t
    result SCM)
  (set
    device-name (scm->locale-string scm-device-name)
    input? (scm->bool scm-input?)
    channel-count (scm->uint32 scm-channel-count)
    sample-rate (scm->uint32 scm-sample-rate)
    latency (scm->uint32 scm-latency))
  (sp-alloc-define-zero sp-port sp-port-t* (sizeof sp-port-t))
  (status-require (sp-alsa-open sp-port device-name input? channel-count sample-rate latency))
  (set result (scm-sp-object-create sp-port sp-object-type-port))
  (label exit
    (status->scm-return result)))
)

(define (sp-guile-init) void
  (declare
    type-slots SCM
    scm-symbol-data SCM)
  scm-c-define-procedure-c-init
  (set
    scm-rnrs-raise (scm-c-public-ref "rnrs exceptions" "raise")
    scm-symbol-data (scm-from-latin1-symbol "data")
    type-slots (scm-list-1 scm-symbol-data)
    scm-type-port (scm-make-foreign-object-type (scm-from-latin1-symbol "sp-port") type-slots 0)
    scm-type-windowed-sinc
    (scm-make-foreign-object-type (scm-from-latin1-symbol "sp-windowed-sinc") type-slots 0))
  (scm-c-define-procedure-c
    "sp-sine!"
    6
    0
    0
    scm-sp-sine!
    "data len sample-duration freq phase amp -> unspecified
    sample-vector integer integer rational rational rational rational")
  (scm-c-define-procedure-c
    "sp-sine-lq!"
    6
    0
    0
    scm-sp-sine-lq!
    "data len sample-duration freq phase amp  -> unspecified
    sample-vector integer integer rational rational rational rational
    faster, lower precision version of sp-sine!.
    currently faster by a factor of about 2.6")
  (scm-c-define-procedure-c
    "sp-convolve!" 4 0 0 scm-sp-convolve! "result a b carryover -> unspecified")
  (scm-c-define-procedure-c
    "sp-windowed-sinc!"
    7
    2
    0
    scm-sp-windowed-sinc!
    "result source previous next sample-rate freq transition [start end] -> unspecified
    f32vector f32vector f32vector f32vector number number integer integer -> boolean")
  (scm-c-define-procedure-c
    "sp-windowed-sinc-state"
    3
    1
    0
    scm-sp-windowed-sinc-state-create
    "sample-rate radian-frequency transition [state] -> state
    rational rational rational [sp-windowed-sinc] -> sp-windowed-sinc")
  (scm-c-define-procedure-c
    "sp-fft"
    1
    0
    0
    scm-sp-fft
    "sample-vector:values-at-times -> sample-vector:frequencies
    discrete fourier transform on the input data")
  (scm-c-define-procedure-c
    "sp-ifft"
    1
    0
    0
    scm-sp-ifft
    "sample-vector:frequencies -> sample-vector:values-at-times
    inverse discrete fourier transform on the input data")
  #;(
  ; check (sizeof sp-sample-t)
  ; chec (sizeof sp-sample-count-t)
  ; chec (sizeof sp-sample-rate-t)
  (scm-c-define-procedure-c
    "f32vector-sum" 1 2 0 scm-f32vector-sum "f32vector [start end] -> number")
  (scm-c-define-procedure-c
    "f64vector-sum" 1 2 0 scm-f64vector-sum "f64vector [start end] -> number")
  (scm-c-define-procedure-c
    "float-nearly-equal?"
    3 0 0 scm-float-nearly-equal?
    "a b margin -> boolean
    number number number -> boolean")

  (scm-c-define-procedure-c "sp-port-close" 1 0 0 scm-sp-port-close "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-input?" 1 0 0 scm-sp-port-input? "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-position?" 1 0 0 scm-sp-port-position? "sp-port -> boolean")
  (scm-c-define-procedure-c
    "sp-port-position" 1 0 0 scm-sp-port-position "sp-port -> integer/boolean")
  (scm-c-define-procedure-c
    "sp-port-channel-count" 1 0 0 scm-sp-port-channel-count "sp-port -> integer")
  (scm-c-define-procedure-c
    "sp-port-sample-rate" 1 0 0 scm-sp-port-sample-rate "sp-port -> integer/boolean")
  (scm-c-define-procedure-c "sp-port?" 1 0 0 scm-sp-port? "sp-port -> boolean")
  (scm-c-define-procedure-c
    "sp-alsa-open"
    5 0 0 scm-sp-alsa-open "device-name input? channel-count sample-rate latency -> sp-port")
  (scm-c-define-procedure-c
    "sp-file-open" 3 0 0 scm-sp-file-open "path channel-count sample-rate -> sp-port")
  (scm-c-define-procedure-c
    "sp-port-write"
    2
    1
    0
    scm-sp-port-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
    write sample data to the channels of port")
  (scm-c-define-procedure-c
    "sp-port-read"
    2 0 0 scm-sp-port-read "sp-port integer:sample-count -> (f32vector ...):channel-data")
  (scm-c-define-procedure-c
    "sp-port-set-position"
    2
    0
    0
    scm-sp-port-set-position
    "sp-port integer:sample-offset -> boolean
    sample-offset can be negative, in which case it is from the end of the port")

  (scm-c-define-procedure-c
    "sp-moving-average!"
    5
    2
    0
    scm-sp-moving-average!
    "result source previous next distance [start end] -> unspecified
  f32vector f32vector f32vector f32vector integer integer integer [integer]")

  ))