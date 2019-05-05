(declare
  (type
    (struct
      (cut-l sp-float-t)
      (cut-h sp-float-t)
      (trn-l sp-float-t)
      (trn-h sp-float-t)
      (is-reject uint8-t)
      (noise sp-sample-t*)
      (temp sp-sample-t*)
      (filter-state sp-convolution-filter-state-t))))

(define (sp-noise-event-f start end out event) (void sp-count-t sp-count-t sp-block-t sp-event-t*)
  (define s sp-noise-event-state-t* event:state)
  ; map with resolution, write filtered result into temp buffer, apply amps when copying into out
  (set s:random-state (sp-random s:random-state (- end start) s:noise))
  (sp-windowed-sinc-bp-br
    s:noise (- end start) s:cut-l s:cut-h s:trn-l s:trn-h is-reject s:filter-state s:temp)
  (sp- out start (- end start) s:config-len s:config s:state))

(define (sp-noise-event start end channel-count config-len config out-event)
  (status-t sp-count-t sp-count-t sp-count-t sp-count-t sp-synth-partial-t* sp-event-t*)
  "memory for event.state will be allocated and then owned by the caller.
  config is copied into event.state"
  status-declare
  (declare
    e sp-event-t
    state sp-synth-event-state-t*)
  (status-require (sph-helper-malloc (* channel-count (sizeof sp-synth-event-state-t)) &state))
  (status-require (sp-synth-state-new channel-count config-len config &state:state))
  (status-require (sph-helper-malloc (* (- end start) (sizeof sp-sample-t)) &noise))
  (memcpy state:config config (* config-len (sizeof sp-synth-partial-t)))
  (set
    state:config-len config-len
    e.start start
    e.end end
    e.f sp-synth-event-f
    e.state state)
  (set *out-event e)
  (label exit
    (return status)))