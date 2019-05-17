(define (sp-phase-96 current change) (sp-count-t sp-count-t sp-count-t)
  "accumulate an integer phase and reset it after cycles.
  float value phases would be harder to reset"
  (declare result sp-count-t)
  (set result (+ current change))
  (return (if* (<= 96000 result) (modulo result 96000) result)))

(define (sp-phase-96-float current change) (sp-count-t sp-count-t double)
  "accumulate an integer phase with change given as a float value.
  change must be a positive value and is rounded to the next larger integer"
  (return (sp-phase-96 current (sp-cheap-ceiling-positive change))))

(define (sp-synth-state-new channel-count config-len config out-state)
  (status-t sp-count-t sp-synth-count-t sp-synth-partial-t* sp-count-t**)
  "contains the initial phase offsets per partial and channel
  as a flat array. should be freed with free"
  status-declare
  (declare i sp-count-t channel-i sp-count-t)
  (status-require (sph-helper-calloc (* channel-count config-len (sizeof sp-count-t)) out-state))
  (for ((set i 0) (< i config-len) (set i (+ 1 i)))
    (for ((set channel-i 0) (< channel-i channel-count) (set channel-i (+ 1 channel-i)))
      (set (array-get *out-state (+ channel-i (* channel-count i)))
        (array-get (struct-get (array-get config i) phs) channel-i))))
  (label exit (return status)))

(define (sp-synth out start duration config-len config phases)
  (status-t sp-block-t sp-count-t sp-count-t sp-synth-count-t sp-synth-partial-t* sp-count-t*)
  "create sines that start and end at specific times and can optionally modulate the frequency of others.
  sp-synth output is summed into out.
  amplitude and wavelength can be controlled by arrays separately for each partial and channel.
  modulators can be modulated themselves in chains. state has to be allocated by the caller with sp-synth-state-new.
  modulator amplitude is relative to carrier wavelength.
  paths are relative to the start of partials.
  # requirements
  * modulators must come after carriers in config
  * config-len must not change between calls with the same state
  * all amplitude/wavelength arrays must be of sufficient size and available for all channels
  * sp-initialise must have been called once before using sp-synth
  # algorithm
  * read config from the end to the start
  * write modulator output to temporary buffers that are indexed by modified partial id
  * apply modulator output from the buffers and sum to output for final carriers
  * each partial has integer phases that are reset in cycles and kept in state between calls"
  ; config is evaluated from last to first.
  ; modulation is duration length, paths are samples relative to the partial start
  status-declare
  (declare
    amp sp-sample-t
    carrier sp-sample-t*
    channel-i sp-count-t
    end sp-count-t
    i sp-count-t
    modulated-wvl sp-sample-t
    modulation-index (array sp-sample-t* (sp-synth-partial-limit sp-channel-limit))
    modulation sp-sample-t*
    phs sp-count-t
    prt-duration sp-count-t
    prt-i sp-synth-count-t
    prt-offset-right sp-count-t
    prt-offset sp-count-t
    prt sp-synth-partial-t
    prt-start sp-count-t
    wvl sp-count-t)
  (sc-comment
    "modulation blocks (channel array + data. at least one is carrier and writes only to output)")
  (memreg-init (* (- config-len 1) out.channels))
  (memset modulation-index 0 (sizeof modulation-index))
  (set end (+ start duration) prt-i config-len)
  (while prt-i
    (set prt-i (- prt-i 1) prt (array-get config prt-i))
    (if (< end prt.start) break)
    (if (<= prt.end start) continue)
    (set
      prt-start (if* (< prt.start start) (- start prt.start) 0)
      prt-offset (if* (> prt.start start) (- prt.start start) 0)
      prt-offset-right (if* (> prt.end end) 0 (- end prt.end))
      prt-duration (- duration prt-offset prt-offset-right))
    (if prt.modifies
      (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
        (set carrier (array-get modulation-index (- prt.modifies 1) channel-i))
        (if (not carrier)
          (begin
            (status-require (sph-helper-calloc (* duration (sizeof sp-sample-t)) &carrier))
            (memreg-add carrier)))
        (set
          phs (array-get phases (+ channel-i (* out.channels prt-i)))
          modulation (array-get modulation-index prt-i channel-i))
        (for ((set i 0) (< i prt-duration) (set i (+ 1 i)))
          (set
            amp (array-get prt.amp channel-i (+ prt-start i))
            (array-get carrier (+ prt-offset i))
            (+ (array-get carrier (+ prt-offset i)) (* amp (sp-sine-96 phs))) wvl
            (array-get prt.wvl channel-i (+ prt-start i)) modulated-wvl
            (if* modulation (+ wvl (* wvl (array-get modulation (+ prt-offset i)))) wvl) phs
            (sp-phase-96-float phs (/ 48000 modulated-wvl))))
        (set
          (array-get phases (+ channel-i (* out.channels prt-i))) phs
          (array-get modulation-index (- prt.modifies 1) channel-i) carrier))
      (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
        (set
          phs (array-get phases (+ channel-i (* out.channels prt-i)))
          modulation (array-get modulation-index prt-i channel-i))
        (for ((set i 0) (< i prt-duration) (set i (+ 1 i)))
          (set
            amp (array-get prt.amp channel-i (+ prt-start i))
            wvl (array-get prt.wvl channel-i (+ prt-start i))
            modulated-wvl
            (if* modulation (+ wvl (* wvl (array-get modulation (+ prt-offset i)))) wvl) phs
            (sp-phase-96-float phs (/ 48000 modulated-wvl))
            (array-get (array-get out.samples channel-i) (+ prt-offset i))
            (+ (array-get (array-get out.samples channel-i) (+ prt-offset i))
              (* amp (sp-sine-96 phs)))))
        (set (array-get phases (+ channel-i (* out.channels prt-i))) phs))))
  (label exit memreg-free (return status)))

(pre-define (sp-synth-partial-set-channel prt channel amp-array wvl-array phs-array)
  (set
    (array-get prt.amp channel) amp-array
    (array-get prt.wvl channel) wvl-array
    (array-get prt.phs channel) phs-array))

(define (sp-synth-partial-1 start end modifies amp wvl phs)
  (sp-synth-partial-t sp-count-t sp-count-t sp-synth-count-t sp-sample-t* sp-count-t* sp-count-t)
  "setup a synth partial with one channel"
  (declare prt sp-synth-partial-t)
  (set prt.start start prt.end end prt.modifies modifies)
  (sp-synth-partial-set-channel prt 0 amp wvl phs)
  (return prt))

(define (sp-synth-partial-2 start end modifies amp1 amp2 wvl1 wvl2 phs1 phs2)
  (sp-synth-partial-t sp-count-t sp-count-t
    sp-synth-count-t sp-sample-t* sp-sample-t* sp-count-t* sp-count-t* sp-count-t sp-count-t)
  "setup a synth partial with two channels"
  (declare prt sp-synth-partial-t)
  (set prt.start start prt.end end prt.modifies modifies)
  (sp-synth-partial-set-channel prt 0 amp1 wvl1 phs1)
  (sp-synth-partial-set-channel prt 1 amp2 wvl2 phs2)
  (return prt))

(define (sp-triangle t a b) (sp-sample-t sp-count-t sp-count-t sp-count-t)
  "return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0"
  (declare remainder sp-count-t)
  (set remainder (modulo t (+ a b)))
  (return
    (if* (< remainder a) (* remainder (/ 1 (convert-type a sp-sample-t)))
      (* (- (convert-type b sp-sample-t) (- remainder (convert-type a sp-sample-t)))
        (/ 1 (convert-type b sp-sample-t))))))

(define (sp-triangle-96 t) (sp-sample-t sp-count-t) (return (sp-triangle t 48000 48000)))

(define (sp-square-96 t) (sp-sample-t sp-count-t)
  (return (if* (< (modulo (* 2 t) (* 2 96000)) 96000) -1 1)))

(define (sp-sine-table-new out size) (status-t sp-sample-t** sp-count-t)
  "writes a sine wave of size into out. can be used as a lookup table"
  status-declare
  (declare i sp-count-t out-array sp-sample-t*)
  (status-require (sph-helper-malloc (* size (sizeof sp-sample-t*)) &out-array))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out-array i) (sin (* i (/ M_PI (/ size 2))))))
  (set *out out-array)
  (label exit (return status)))