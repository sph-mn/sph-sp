(pre-define (arrays-template-h value-t value-type-name type-name)
  (declare
    ((pre-concat sp_ value-type-name _sort-swap) a b c) (void void* ssize-t ssize-t)
    ((pre-concat sp_ value-type-name _sort-less) a b c) (uint8-t void* ssize-t ssize-t)
    ((pre-concat sp_ value-type-name _round-to-multiple) a base) (value-t value-t value-t)
    ((pre-concat sp_ type-name _absolute-max) in count) (value-t value-t* sp-size-t)
    ((pre-concat sp_ type-name _reverse) in count out) (void value-t* sp-size-t value-t*)
    ((pre-concat sp_ type-name _equal) in count value) (sp-bool-t value-t* sp-size-t value-t)
    ((pre-concat sp_ type-name _square) in count) (void value-t* sp-size-t)
    ((pre-concat sp_ type-name _add) in-out count value) (void value-t* sp-size-t value-t)
    ((pre-concat sp_ type-name _multiply) in-out count value) (void value-t* sp-size-t value-t)
    ((pre-concat sp_ type-name _divide) in-out count value) (void value-t* sp-size-t value-t)
    ((pre-concat sp_ type-name _set) in-out count value) (void value-t* sp-size-t value-t)
    ((pre-concat sp_ type-name _subtract) in-out count value) (void value-t* sp-size-t value-t)
    ((pre-concat sp_ type-name _new) count out) (status-t sp-size-t value-t**)
    ((pre-concat sp_ type-name _copy) in count out) (void value-t* sp-size-t value-t*)
    ((pre-concat sp_ type-name _cusum) in count out) (void value-t* value-t value-t*)
    ((pre-concat sp_ type-name _swap) in-out index-1 index-2) (void sp-time-t* sp-ssize-t sp-ssize-t)
    ((pre-concat sp_ type-name _shuffle) in count) (void value-t* sp-size-t)
    ((pre-concat sp_ type-name _array-free) in count) (void value-t** sp-size-t)
    ((pre-concat sp_ type-name _additions) start summand count out)
    (void value-t value-t value-t value-t*)
    ((pre-concat sp_ type-name _duplicate) a count out) (status-t value-t* sp-size-t value-t**)
    ((pre-concat sp_ type-name _range) in start end out) (void value-t* sp-size-t sp-size-t value-t*)
    ((pre-concat sp_ type-name _and_ type-name) a b count limit out)
    (void value-t* value-t* sp-size-t value-t value-t*)
    ((pre-concat sp_ type-name _or_ type-name) a b count limit out)
    (void value-t* value-t* sp-size-t value-t value-t*)
    ((pre-concat sp_ type-name _xor_ type-name) a b count limit out)
    (void value-t* value-t* sp-size-t value-t value-t*)
    ((pre-concat sp_ type-name _multiply_ type-name) in-out count in)
    (void value-t* sp-size-t value-t*)
    ((pre-concat sp_ type-name _divide_ type-name) in-out count in)
    (void value-t* sp-size-t value-t*)
    ((pre-concat sp_ type-name _add_ type-name) in-out count in) (void value-t* sp-size-t value-t*)
    ((pre-concat sp_ type-name _subtract_ type-name) in-out count in)
    (void value-t* sp-size-t value-t*)))