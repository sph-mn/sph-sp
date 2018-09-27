/* memreg registers memory in a local variable, for example to free all memory allocated at point.
the variables memreg_register and memreg_index will also be available.
usage:
     memreg_init(4);
     memreg_add(&variable-1);
     memreg_add(&variable-2);
     memreg_free; */
#define memreg_init(register_size) \
  void* memreg_register[register_size]; \
  unsigned int memreg_index; \
  memreg_index = 0
/** add a pointer to the register. does not protect against buffer overflow */
#define memreg_add(address) \
  memreg_register[memreg_index] = address; \
  memreg_index = (1 + memreg_index)
#define memreg_free \
  while (memreg_index) { \
    /* free all currently registered pointers */ \
    memreg_index = (memreg_index - 1); \
    free((*(memreg_register + memreg_index))); \
  }
/* the *_named variant of memreg supports multiple concurrent registers identified by name
usage:
     memreg_init_named(testname, 4);
     memreg_add_named(testname, &variable);
     memreg_free_named(testname); */
#define memreg_init_named(register_id, register_size) \
  void* memreg_register##_##register_id[register_size]; \
  unsigned int memreg_index##_##register_id; \
  memreg_index##_##register_id = 0
/** does not protect against buffer overflow */
#define memreg_add_named(register_id, address) \
  memreg_register##_##register_id[memreg_index##_##register_id] = address; \
  memreg_index##_##register_id = (1 + memreg_index##_##register_id)
#define memreg_free_named(register_id) \
  while (memreg_index##_##register_id) { \
    memreg_index##_##register_id = (memreg_index##_##register_id - 1); \
    free((*(memreg_register##_##register_id + memreg_index##_##register_id))); \
  }
