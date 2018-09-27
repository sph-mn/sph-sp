(sc-comment
  "memreg registers memory in a local variable, for example to free all memory allocated at point."
  "the variables memreg_register and memreg_index will also be available."
  "usage:
     memreg_init(4);
     memreg_add(&variable-1);
     memreg_add(&variable-2);
     memreg_free;")

(pre-define
  (memreg-init register-size)
  (begin
    (declare
      memreg-register (array void* (register-size))
      memreg-index (unsigned int))
    (set memreg-index 0))
  (memreg-add address)
  (begin
    "add a pointer to the register. does not protect against buffer overflow"
    (set
      (array-get memreg-register memreg-index) address
      memreg-index (+ 1 memreg-index)))
  memreg-free
  (while memreg-index
    (sc-comment "free all currently registered pointers")
    (set memreg-index (- memreg-index 1))
    (free (pointer-get (+ memreg-register memreg-index)))))

(sc-comment
  "the *_named variant of memreg supports multiple concurrent registers identified by name"
  "usage:
     memreg_init_named(testname, 4);
     memreg_add_named(testname, &variable);
     memreg_free_named(testname);")

(pre-define
  (memreg-init-named register-id register-size)
  (begin
    (declare
      (pre-concat memreg-register _ register-id) (array void* (register-size))
      (pre-concat memreg-index _ register-id) (unsigned int))
    (set (pre-concat memreg-index _ register-id) 0))
  (memreg-add-named register-id address)
  (begin
    "does not protect against buffer overflow"
    (set
      (array-get (pre-concat memreg-register _ register-id) (pre-concat memreg-index _ register-id))
      address (pre-concat memreg-index _ register-id) (+ 1 (pre-concat memreg-index _ register-id))))
  (memreg-free-named register-id)
  (while (pre-concat memreg-index _ register-id)
    (set (pre-concat memreg-index _ register-id) (- (pre-concat memreg-index _ register-id) 1))
    (free
      (pointer-get
        (+ (pre-concat memreg-register _ register-id) (pre-concat memreg-index _ register-id))))))