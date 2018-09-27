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
(pre-define (scm-sp-port a) (convert-type (scm-sp-object-data a) sp-port-t*))

(define (scm-sp-port? a) (SCM SCM)
  (return
    (scm-from-bool
      (and (SCM-SMOB-PREDICATE scm-type-sp-object a) (= sp-object-type-port (scm-sp-object-type a))))))

(define (scm-sp-port-channel-count port) (SCM SCM)
  (return (scm-from-uint32 (: (scm-sp-port port) channel-count))))

(define (scm-sp-port-sample-rate port) (SCM SCM)
  (return (scm-from-uint32 (: (scm-sp-port port) sample-rate))))

(define (scm-sp-port-position? port) (SCM SCM)
  (return (scm-from-bool (bit-and sp-port-bit-position (: (scm-sp-port port) flags)))))

(define (scm-sp-port-input? port) (SCM SCM)
  (return (scm-from-bool (bit-and sp-port-bit-input (: (scm-sp-port port) flags)))))

(define (scm-sp-port-position port) (SCM SCM)
  "returns the current port position in number of octets"
  (declare position size-t)
  (sp-port-position &position (scm-sp-port port))
  (return (scm-from-size-t position)))

(define (scm-sp-port-close a) (SCM SCM)
  status-declare
  (set status (sp-port-close (scm-sp-port a)))
  (status->scm-return SCM-UNSPECIFIED))

(define (scm-sp-port-read scm-port scm-sample-count) (SCM SCM SCM)
  status-declare
  (declare
    port sp-port-t*
    sample-count uint32-t
    channel-count uint32-t
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
    channel-count uint32-t
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

(define (scm-sp-port-set-position scm-port scm-sample-offset) (SCM SCM SCM)
  status-declare
  (set status (sp-port-set-position (scm-sp-port scm-port) (scm->uint64 scm-sample-offset)))
  (status->scm-return SCM-UNSPECIFIED))

(define (scm-sp-file-open scm-path scm-channel-count scm-sample-rate) (SCM SCM SCM SCM)
  status-declare
  (declare
    path uint8-t*
    channel-count uint32-t
    sample-rate uint32-t
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
    channel-count uint32-t
    sample-rate uint32-t
    latency uint32-t
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

(define (scm-sp-convolve! result a b carryover carryover-len) (SCM SCM SCM SCM SCM SCM)
  (declare
    a-len uint32-t
    b-len uint32-t)
  (set
    a-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH a))
    b-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH b)))
  (sp-convolve
    (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*)
    a-len
    (convert-type (SCM-BYTEVECTOR-CONTENTS b) sp-sample-t*)
    b-len (convert-type (SCM-BYTEVECTOR-CONTENTS carryover) sp-sample-t*) (scm->size-t carryover-len))
  (return SCM-UNSPECIFIED))

(define (scm-sp-moving-average! result source scm-prev scm-next distance start end)
  (SCM SCM SCM SCM SCM SCM SCM SCM)
  (declare
    source-len uint32-t
    prev sp-sample-t*
    prev-len uint32-t
    next sp-sample-t*
    next-len uint32-t)
  (set source-len (sp-octets->samples (SCM-BYTEVECTOR-LENGTH source)))
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
  (declare state sp-windowed-sinc-state-t*)
  (if (scm-is-true old-state)
    (set state (convert-type (scm-sp-object-data old-state) sp-windowed-sinc-state-t*))
    (set state 0))
  (sp-windowed-sinc-state-create
    (scm->uint32 sample-rate) (scm->double freq) (scm->double transition) &state)
  ; old-state has either been updated or a new state been created
  (return
    (if* (scm-is-true old-state) old-state
      (scm-sp-object-create state sp-object-type-windowed-sinc))))

(define (scm-sp-windowed-sinc! result source state) (SCM SCM SCM SCM)
  ;(define source-len uint32-t (sp-octets->samples (SCM-BYTEVECTOR-LENGTH source)))
  #;(define
    prev sp-sample-t*
    prev-len uint32-t
    next sp-sample-t*
    next-len uint32-t)
  ;(define state sp-windowed-sinc-state-t* (scm-sp-object-data state))
  #;(sp-windowed-sinc (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) source-len
    (optional-index start 0) (optional-index end (- source-len 1)) state)
  (return SCM-UNSPECIFIED))

(define (scm-sp-fft source) (SCM SCM)
  status-declare
  (declare
    result-len uint32-t
    result SCM)
  (set
    result-len (/ (* 3 (SCM-BYTEVECTOR-LENGTH source)) 2)
    result (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require
    (sp-fft
      (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
      result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit
    (status->scm-return result)))

(define (scm-sp-ifft source) (SCM SCM)
  status-declare
  (declare
    result-len uint32-t
    result SCM)
  (set
    result-len (* (- (SCM-BYTEVECTOR-LENGTH source) 1) 2)
    result (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require
    (sp-ifft
      (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
      result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit
    (status->scm-return result)))

(define (sp-guile-init) void
  (declare
    type-slots SCM
    scm-symbol-data SCM)
  (set
    scm-symbol-data (scm-from-latin1-symbol "data")
    type-slots (scm-list-1 scm-symbol-data)
    scm-type-port (scm-make-foreign-object-type (scm-from-latin1-symbol "sp-port") type-slots 0)
    scm-type-windowed-sinc
    (scm-make-foreign-object-type (scm-from-latin1-symbol "sp-windowed-sinc") type-slots 0))
  scm-type-txn
  (scm-make-foreign-object-type (scm-from-latin1-symbol "db-txn") type-slots 0)
  (init-sp-io)
  (init-sp-generate)
  (init-sp-transform)
  scm-c-define-procedure-c-init
  (set scm-rnrs-raise (scm-c-public-ref "rnrs exceptions" "raise"))
  (scm-c-define-procedure-c
    "f32vector-sum" 1 2 0 scm-f32vector-sum "f32vector [start end] -> number")
  (scm-c-define-procedure-c
    "f64vector-sum" 1 2 0 scm-f64vector-sum "f64vector [start end] -> number")
  (scm-c-define-procedure-c
    "float-nearly-equal?"
    3 0 0 scm-float-nearly-equal?
    "a b margin -> boolean
    number number number -> boolean")
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
    currently faster by a factor of about 2.6")
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