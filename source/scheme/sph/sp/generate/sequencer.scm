(library (sph sp generate sequencer)
  (export
    seq
    seq-default-mixer
    seq-event
    seq-event-custom
    seq-event-f
    seq-event-groups
    seq-event-list->events
    seq-event-name
    seq-event-new
    seq-event-start
    seq-event-update
    seq-events-merge
    seq-index-data
    seq-index-end
    seq-index-events
    seq-index-f
    seq-index-f-new
    seq-index-i-f
    seq-index-i-f-new
    seq-index-i-next
    seq-index-new
    seq-index-next
    seq-index-start
    seq-index-update
    seq-output
    seq-output-new
    seq-state-add-events
    seq-state-custom
    seq-state-events-custom
    seq-state-index
    seq-state-index-i
    seq-state-input
    seq-state-new
    seq-state-options
    seq-state-output
    seq-state-update)
  (import
    (guile)
    (sph)
    (sph alist)
    (sph list)
    (sph list one)
    (sph number)
    (sph sp generate)
    (sph vector)
    (only (srfi srfi-1) zip partition))

  (define sph-sp-generate-sequencer-description
    "seq calls sample generating functions at customisable times, shares state values between event-f and
     mixes output values to create a single result. seq emphasizes the \"when\", event-f the \"what\".
     seq can return single sample values or sample arrays, this depends completely on the user supplied events-f and event-f functions,
     as well as the time intervals seq is called.
     data
       event-f: a function that returns a sample or segment
       event: a event-f and information when it should be called
       index: a vector that stores lists of events for a time range
       events: (event-list ...)
       event-list: (event ...)
       event-custom: the initial custom values for event-f
       events-custom: ((event-name . output-custom) ...)
     process
       seq is called for each sample or segment with the time as argument. if no valid index is loaded,
       events-f is called to get the next event-list.
       seq checks all relevant event-f in index for if they need to be called now, eventually does it
       and stores results with sample values and possibly custom values in an output structure that is
       available to all subsequently called event-f for this seq call so that event-f can modulate other event-f.
       event-f can be ordered and stored in priority groups
       events contains multiple event-lists who are executed from left to right, for a simpler way to control execute-order priority.
       events continue to be called until the associated event-f returns false
     other
       event-f have a dedicated state that they can update
       event-f can add events
       event-f states and output values are accessible to other event-f. for example for modulation
       events are repeated indefinitely until the associated event-f returns false, so that the duration is easier to modulate
       only a function to create a list of events needs to be supplied
       events are indexed by time to reduce the amount of events that have to be checked
       new events are automatically added to the index
       purely functional
       the default mixer sums all output values with rounding error compensation and clipping
       replaceable mixer function
       sample resolution for timings")

  (define seq-index-data (vector-accessor 0))
  (define seq-index-end (vector-accessor 1))
  (define seq-index-events (vector-accessor 2))
  (define seq-index-f (vector-accessor 3))
  (define seq-index-i-f (vector-accessor 4))
  (define seq-index-start (vector-accessor 5))
  (define (seq-index-new data end events f i-f start) (vector data end events f i-f start))

  (define* (seq-index-update a #:key data end events f i-f start)
    (vector (or data (seq-index-data a)) (or end (seq-index-end a))
      (or events (seq-index-events a)) (or f (seq-index-f a))
      (or i-f (seq-index-i-f a)) (or start (seq-index-start a))))

  (define (seq-index-next index time state)
    (let (index-f (seq-index-f index))
      (if index-f (index-f time state) (seq-index-f-new time state))))

  (define (seq-index-i-next index time) ((seq-index-i-f index) time (seq-index-start index)))

  (define (seq-index-add-events index events) "seq-index seq-events"
    (if (null? events) events
      (seq-index-update index #:events (seq-events-merge (seq-index-events index) events))))

  (define (seq-index-i-f-new duration index-size)
    "vector number number -> procedure
     return a procedure that returns an index in index-data for a time value"
    (let (slot-duration (/ duration index-size))
      (l (time index-start) (inexact->exact (truncate (/ (- time index-start) slot-duration))))))

  (define (seq-index-f-new time state)
    "number seq-state -> procedure
     create a procedure that returns new event indexes for a time and state using
     the procedure events-f from the state to get events"
    (let*
      ( (options (seq-state-options state)) (duration (alist-ref-q options index-duration))
        (size (inexact->exact (round (* duration (alist-ref-q options index-size-factor)))))
        (index-i-f (seq-index-i-f-new duration size)) (events-f (seq-state-events-f state)))
      (letrec
        ( (index-f
            (l (time state) "number seq-state -> seq-index"
              ; create an alist where the key is the target index and the value are events for this index.
              ; then add empty entries for intermediate indexes, remove keys and create an index-data vector
              ; and the index-events list for out-of-index events
              (let*
                ( (end (+ time duration)) (events-1 (events-f time end state))
                  (events-2
                    (if (seq-state-index state) (seq-index-events (seq-state-index state)) null))
                  (slots
                    (group (append events-1 events-2)
                      (l (a)
                        ; use zero for events outside index
                        (let (index-i (index-i-f (seq-event-start a) time))
                          (if (< index-i size) (+ 1 index-i) 0)))))
                  (slots (list-sort-with-accessor < first slots))
                  (slots
                    (let loop ((rest slots) (last-id 0))
                      ; add empty entries for inbetween slots
                      (if (null? rest) rest
                        (let* ((a (first rest)) (id (first a)) (diff (- id last-id)))
                          (if (> diff 1)
                            (append (make-list (- diff 1) (list null))
                              (pair a (loop (tail rest) id)))
                            (pair a (loop (tail rest) id))))))))
                (if (null? slots) (seq-index-new (vector) end null index-f index-i-f time)
                  (let*
                    ( (slots (if (zero? (first (first slots))) slots (pair (list 0) slots)))
                      (events-1 (map tail (tail slots))) (events-2 (tail (first slots)))
                      (events-1-length (length events-1))
                      (events-1
                        ; add empty entries for trailing slots
                        (if (< events-1-length size)
                          (append events-1 (make-list (- size events-1-length) null)) events-1)))
                    (seq-index-new (apply vector events-1) end events-2 index-f index-i-f time)))))))
        index-f)))

  (define-as seq-state-options-default alist-q index-duration 4 index-size-factor 4)
  (define seq-state-custom (vector-accessor 0))
  (define seq-state-events-f (vector-accessor 1))
  (define seq-state-events-custom (vector-accessor 2))
  (define seq-state-index (vector-accessor 3))
  (define seq-state-index-i (vector-accessor 4))
  (define seq-state-input (vector-accessor 5))
  (define seq-state-mixer (vector-accessor 6))
  (define seq-state-options (vector-accessor 7))
  (define seq-state-output (vector-accessor 8))

  (define*
    (seq-state-new events-f #:key events-custom custom index index-i input mixer options output)
    "procedure [#:event-f-list list #:custom alist] -> seq-state
     create a new state object.
     seq-state: (results event-f-list index-i index index-f custom)
     index-f: -> (procedure:index-i-f . vector:index)
     custom: ((symbol . any) ...)
     event-f-list: list"
    (vector (or custom null) events-f
      (or events-custom null) index
      (or index-i 0) (or input null)
      (or mixer seq-default-mixer)
      (or (and options (alist-merge seq-state-options-default options)) seq-state-options-default)
      (or output null)))

  (define*
    (seq-state-update a #:key custom events-f events-custom index index-i input mixer options
      output)
    (vector (or custom (seq-state-custom a)) (or events-f (seq-state-events-f a))
      (or events-custom (seq-state-events-custom a)) (or index (seq-state-index a))
      (or index-i (seq-state-index-i a)) (or input (seq-state-input a))
      (or mixer (seq-state-mixer a)) (or options (seq-state-options a))
      (or output (seq-state-output a))))

  (define seq-event-custom (vector-accessor 0))
  (define seq-event-f (vector-accessor 1))
  (define seq-event-groups (vector-accessor 2))
  (define seq-event-name (vector-accessor 3))
  (define seq-event-start (vector-accessor 4))

  (define* (seq-event-new f #:optional name start custom groups)
    "procedure #:key integer (symbol ...) any -> vector"
    (vector (or custom null) f groups (or name (q unnamed)) (or start 0)))

  (define-syntax-rule (seq-event name f optional ...) (seq-event-new f (q name) optional ...))

  (define* (seq-event-update a #:key f start name groups custom)
    (vector (or start (seq-event-start a)) (or f (seq-event-f a))
      (or name (seq-event-name a)) (or groups (seq-event-groups a)) (or custom (seq-event-custom a))))

  (define (seq-event-list->events a) (if (null? a) a (if (vector? (first a)) (map list a) a)))

  (define (seq-events-merge a b) "events:target events:source -> events"
    (if (null? b) a
      (pair (if (null? a) (first b) (append (first a) (first b)))
        (seq-events-merge (if (null? a) a (tail a)) (tail b)))))

  (define (seq-state-add-events state . events)
    "seq-state seq-event-list/seq-events ... -> state
     add new events to either the current input list or the index"
    (let*
      ( (index (seq-state-index state))
        (events
          (map
            (let (index-end (seq-index-end index))
              (l (event-list)
                (apply-values pair (partition (l (a) (< (seq-event-start a) index-end)) event-list))))
            (seq-event-list->events events))))
      (let ((soon (map first events)) (later (map tail events)))
        (seq-state-update state #:input
          (seq-events-merge (seq-state-input state) soon) #:index (seq-index-add-events index later)))))

  (define seq-output-name (vector-accessor 0))
  (define seq-output-data (vector-accessor 1))
  (define seq-output-custom (vector-accessor 2))
  (define seq-output-event (vector-accessor 3))
  (define* (seq-output-new name data custom event) (vector name data custom event))

  (define* (seq-output data state #:optional custom)
    "symbol integer/vector/any seq-state list list:alist -> list
     create the output structure that event-f must return"
    (pairs data state (or custom null)))

  (define (seq-default-mixer output)
    "combines multiple event-f results into one seq result, which should be sample values.
     does not handle arrays"
    (map sp-clip (map-apply float-sum (apply zip (map (compose any->list seq-output-data) output)))))

  (define seq
    (letrec
      ( (index-next
          (l (time state)
            "-> seq-state
            call index-f to get the next index and add the
            input event-f from the next index"
            (let (index (seq-state-index state))
              (and-let*
                ( (index
                    (if index (seq-index-next index time state)
                      ((seq-index-f-new time state) time state)))
                  (data (seq-index-data index))
                  (input
                    (if (zero? (vector-length data)) null
                      (seq-event-list->events (vector-first data)))))
                (seq-state-update state #:input
                  (seq-events-merge (seq-state-input state) input) #:index index #:index-i 0)))))
        (index-ensure
          (l (time state)
            "-> seq-state
            ensure that current index data is available and eventually call index-f to get it"
            (let (index (seq-state-index state)) (if index state (index-next time state)))))
        (input-extend
          (l (state index index-i)
            "-> seq-state
            add event-f from index to the current input list"
            (let (input (seq-event-list->events (vector-ref (seq-index-data index) index-i)))
              (seq-state-update state #:input
                (seq-events-merge (seq-state-input state) input) #:index-i index-i))))
        (index-advance
          (l (time state)
            "-> seq-state
            eventually update the input event-f list from the current or a new index"
            (let (index (seq-state-index state))
              (let*
                ((index-i (seq-state-index-i state)) (index-i-new (seq-index-i-next index time)))
                (if (= index-i-new index-i) state
                  (if (< time (seq-index-end index)) (input-extend state index index-i-new)
                    (index-next time state)))))))
        (clear-output (l (state) (seq-state-update state #:output null)))
        (execute-event-list
          (l (time state event-list events-custom c)
            "number seq-state event-list procedure:{state event-list -> any:result} -> any
            check every event for if it is due and eventually execute it, update state from its
            result and remove it from the event-list if its event-f returns false"
            (if (null? event-list) (c state event-list events-custom)
              ; for each event in input event list
              (let loop
                ( (state state) (rest event-list) (result-event-list null)
                  (result-events-custom null))
                (if (null? rest) (c state result-event-list result-events-custom)
                  (let (event (first rest))
                    ; check if event is due
                    (if (<= (seq-event-start event) time)
                      (let*
                        ( (event-name (seq-event-name event))
                          (event-result
                            ( (seq-event-f event) time state
                              event (- time (seq-event-start event))
                              (or (alist-ref events-custom event-name) (seq-event-custom event)))))
                        (if event-result
                          (list-let event-result (data state . custom)
                            (loop
                              (seq-state-update state #:output
                                (pair (seq-output-new event-name data custom event)
                                  (seq-state-output state)))
                              (tail rest) (pair event result-event-list)
                              (pair (pair event-name custom) result-events-custom)))
                          (loop state (tail rest) result-event-list result-events-custom)))
                      (loop state (tail rest) (pair event result-event-list) result-events-custom))))))))
        (execute-events
          (l (time state)
            "number seq-state -> seq-state
            execute event-lists, update state and eventually leave out empty result event-lists"
            (let loop
              ( (rest (seq-state-input state)) (state state) (result-events null)
                (result-events-custom null))
              (if (null? rest)
                (seq-state-update state #:input
                  (reverse result-events) #:events-custom result-events-custom)
                (execute-event-list time state
                  (first rest) (seq-state-events-custom state)
                  (l (state event-list events-custom)
                    (loop (tail rest) state
                      (if (null? event-list) result-events (pair event-list result-events))
                      (append result-events-custom events-custom)))))))))
      (l (time state c) "integer list -> integer/vector:sample-data list:state"
        (or
          (and-let*
            ( (state (index-ensure time state)) (state (index-advance time state))
              (state (execute-events time state)))
            (c ((seq-state-mixer state) (seq-state-output state)) (clear-output state)))
          (c 0 state))))))
