
#ifndef sph_memory
#define sph_memory

#include <stdlib.h>
#include <stdio.h>
#include <sph/memory.h>
uint8_t* sph_memory_status_description(status_t a) {
  uint8_t* b;
  if (sph_memory_status_id_memory == a.id) {
    b = "not enough memory or other memory allocation error";
  } else {
    b = "";
  };
}
uint8_t* sph_memory_status_name(status_t a) {
  uint8_t* b;
  if (sph_memory_status_id_memory == a.id) {
    b = "memory";
  } else {
    b = "unknown";
  };
}
status_t sph_memory_malloc(size_t size, void** result) {
  status_declare;
  void* a;
  a = malloc(size);
  if (a) {
    *result = a;
  } else {
    sph_memory_error;
  };
exit:
  status_return;
}

/** like sph_malloc but allocates one extra byte that is set to zero */
status_t sph_memory_malloc_string(size_t length, uint8_t** result) {
  status_declare;
  uint8_t* a;
  status_require((sph_malloc((1 + length), (&a))));
  a[length] = 0;
  *result = a;
exit:
  status_return;
}
status_t sph_memory_calloc(size_t size, void** result) {
  status_declare;
  void* a;
  a = calloc(size, 1);
  if (a) {
    *result = a;
  } else {
    sph_memory_error;
  };
exit:
  status_return;
}
status_t sph_memory_realloc(size_t size, void** memory) {
  status_declare;
  void* a = realloc((*memory), size);
  if (a) {
    *memory = a;
  } else {
    sph_memory_error;
  };
exit:
  status_return;
}

/** event memory addition with automatic array expansion */
status_t sph_memory_add_with_handler(sph_memory_t* a, void* address, void (*handler)(void*)) {
  status_declare;
  status_require((sph_memory_ensure(a, 4)));
  sph_memory_add_directly(a, address, handler);
exit:
  status_return;
}

/** free all registered memory and unitialize the event-memory register */
void sph_memory_destroy(sph_memory_t* a) {
  if (!a->data) {
    return;
  };
  memreg2_t m;
  for (size_t i = 0; (i < a->used); i += 1) {
    m = sph_array_get((*a), i);
    (m.handler)((m.address));
  };
  sph_memory_free(a);
  a->data = 0;
}
#endif
