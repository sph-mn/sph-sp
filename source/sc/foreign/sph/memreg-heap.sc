(sc-comment "depends on i_array.c."
  "similar to memreg.c but uses a heap allocated array as the memory register
   that can be passed between functions"
  "usage:
     memreg_register_t allocations;
     if(!memreg_heap_new(2, &allocations)) { return(1); }
     memreg_heap_add(allocations, &variable-1);
     memreg_heap_add(allocations, &variable-2);
     memreg_heap_free(allocations);")

(i-array-declare-type memreg-register void*)

(pre-define
  memreg-heap-add i-array-add
  memreg-heap-free-register i-array-free
  (memreg-heap-declare variable-name)
  (begin
    "makes sure that values are null and free succeeds even if not allocated yet"
    (i-array-declare variable-name memreg-register-t))
  (memreg-heap-new register-size register-address)
  (begin
    "true on success, false on failure (failed memory allocation)"
    (memreg-register-new register-size register-address))
  (memreg-heap-free-pointers reg)
  (begin
    "free only the registered memory"
    (while (i-array-in-range reg) (free (i-array-get reg)) (i-array-forward reg)))
  (memreg-heap-free reg)
  (begin
    "memreg-register-t -> unspecified
    free all currently registered pointers and the register array"
    (memreg-heap-free-pointers reg)
    (memreg-heap-free-register reg)))