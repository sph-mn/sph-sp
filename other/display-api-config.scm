; configuration used by exe/display-api.

(add
  ; additional expressions to include. for example for preprocessor generated identifiers
  (declare sp-samples-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-times-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-path-segments-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-events-t (type (struct (data void*) (size size-t) (used size-t) (current size-t)))
    (sp-event-memory-add event address) (status-t sp-event-t* void*)
    (sp-event-memory-fixed-add event address) (status-t sp-event-t* void*)
    (sp-sample-sort-swap a b c) (void void* ssize_t ssize_t)
    (sp-sample-sort-less a b c) (uint8_t void* ssize_t ssize_t)
    (sp-sample-round-to-multiple a base) (sp_sample_t sp_sample_t sp_sample_t)
    (sp-sample-min in count) (sp_sample_t sp_sample_t* sp_size_t)
    (sp-sample-max in count) (sp_sample_t sp_sample_t* sp_size_t)
    (sp-sample-absolute-max in count) (sp_sample_t sp_sample_t* sp_size_t)
    (sp-sample-reverse in count out) (void sp_sample_t* sp_size_t sp_sample_t*)
    (sp-sample-equal in count value) (sp_bool_t sp_sample_t* sp_size_t sp_sample_t)
    (sp-sample-square in count) (void sp_sample_t* sp_size_t)
    (sp-sample-add in-out count value) (void sp_sample_t* sp_size_t sp_sample_t)
    (sp-sample-multiply in-out count value) (void sp_sample_t* sp_size_t sp_sample_t)
    (sp-sample-divide in-out count value) (void sp_sample_t* sp_size_t sp_sample_t)
    (sp-sample-set in-out count value) (void sp_sample_t* sp_size_t sp_sample_t)
    (sp-sample-subtract in-out count value) (void sp_sample_t* sp_size_t sp_sample_t)
    (sp-sample-new count out) (status_t sp_size_t sp_sample_t**)
    (sp-sample-copy in count out) (void sp_sample_t* sp_size_t sp_sample_t*)
    (sp-sample-cusum in count out) (void sp_sample_t* sp_sample_t sp_sample_t*)
    (sp-sample-swap in-out index-1 index-2) (void sp_time_t* sp_ssize_t sp_ssize_t)
    (sp-sample-shuffle in count) (void sp_sample_t* sp_size_t)
    (sp-sample-array-free in count) (void sp_sample_t** sp_size_t)
    (sp-sample-additions start summand count out)
    (void sp_sample_t sp_sample_t sp_sample_t sp_sample_t*) (sp-sample-duplicate a count out)
    (status_t sp_sample_t* sp_size_t sp_sample_t**) (sp-sample-range in start end out)
    (void sp_sample_t* sp_size_t sp_size_t sp_sample_t*) (sp-sample-and-sample a b count limit out)
    (void sp_sample_t* sp_sample_t* sp_size_t sp_sample_t sp_sample_t*)
    (sp-sample-or-sample a b count limit out)
    (void sp_sample_t* sp_sample_t* sp_size_t sp_sample_t sp_sample_t*)
    (sp-sample-xor-sample a b count limit out)
    (void sp_sample_t* sp_sample_t* sp_size_t sp_sample_t sp_sample_t*)
    (sp-sample-multiply-sample in-out count in) (void sp_sample_t* sp_size_t sp_sample_t*)
    (sp-sample-divide-sample in-out count in) (void sp_sample_t* sp_size_t sp_sample_t*)
    (sp-sample-add-sample in-out count in) (void sp_sample_t* sp_size_t sp_sample_t*)
    (sp-sample-subtract-sample in-out count in) (void sp_sample_t* sp_size_t sp_sample_t*)
    (sp-time-sort-swap a b c) (void void* ssize_t ssize_t)
    (sp-time-sort-less a b c) (uint8_t void* ssize_t ssize_t)
    (sp-time-round-to-multiple a base) (sp_time_t sp_time_t sp_time_t)
    (sp-time-min in count) (sp_time_t sp_time_t* sp_size_t)
    (sp-time-max in count) (sp_time_t sp_time_t* sp_size_t)
    (sp-time-absolute-max in count) (sp_time_t sp_time_t* sp_size_t)
    (sp-time-reverse in count out) (void sp_time_t* sp_size_t sp_time_t*)
    (sp-time-equal in count value) (sp_bool_t sp_time_t* sp_size_t sp_time_t)
    (sp-time-square in count) (void sp_time_t* sp_size_t)
    (sp-time-add in-out count value) (void sp_time_t* sp_size_t sp_time_t)
    (sp-time-multiply in-out count value) (void sp_time_t* sp_size_t sp_time_t)
    (sp-time-divide in-out count value) (void sp_time_t* sp_size_t sp_time_t)
    (sp-time-set in-out count value) (void sp_time_t* sp_size_t sp_time_t)
    (sp-time-subtract in-out count value) (void sp_time_t* sp_size_t sp_time_t)
    (sp-time-new count out) (status_t sp_size_t sp_time_t**)
    (sp-time-copy in count out) (void sp_time_t* sp_size_t sp_time_t*)
    (sp-time-cusum in count out) (void sp_time_t* sp_time_t sp_time_t*)
    (sp-time-swap in-out index-1 index-2) (void sp_time_t* sp_ssize_t sp_ssize_t)
    (sp-time-shuffle in count) (void sp_time_t* sp_size_t)
    (sp-time-array-free in count) (void sp_time_t** sp_size_t)
    (sp-time-additions start summand count out) (void sp_time_t sp_time_t sp_time_t sp_time_t*)
    (sp-time-duplicate a count out) (status_t sp_time_t* sp_size_t sp_time_t**)
    (sp-time-range in start end out) (void sp_time_t* sp_size_t sp_size_t sp_time_t*)
    (sp-time-and-time a b count limit out)
    (void sp_time_t* sp_time_t* sp_size_t sp_time_t sp_time_t*) (sp-time-or-time a b count limit out)
    (void sp_time_t* sp_time_t* sp_size_t sp_time_t sp_time_t*)
    (sp-time-xor-time a b count limit out)
    (void sp_time_t* sp_time_t* sp_size_t sp_time_t sp_time_t*)
    (sp-time-multiply-time in-out count in) (void sp_time_t* sp_size_t sp_time_t*)
    (sp-time-divide-time in-out count in) (void sp_time_t* sp_size_t sp_time_t*)
    (sp-time-add-time in-out count in) (void sp_time_t* sp_size_t sp_time_t*)
    (sp-time-subtract-time in-out count in) (void sp_time_t* sp_size_t sp_time_t*)))

(remove "__*")
