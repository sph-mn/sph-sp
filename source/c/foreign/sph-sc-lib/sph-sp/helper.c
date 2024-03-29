
#include <stdlib.h>
#include <stdio.h>
#include <sph/helper.h>
uint8_t* sph_helper_status_description(status_t a) {
  uint8_t* b;
  if (sph_helper_status_id_memory == a.id) {
    b = "not enough memory or other memory allocation error";
  } else {
    b = "";
  };
}
uint8_t* sph_helper_status_name(status_t a) {
  uint8_t* b;
  if (sph_helper_status_id_memory == a.id) {
    b = "memory";
  } else {
    b = "unknown";
  };
}

/** allocation helpers use status-t and have the same interface */
status_t sph_helper_primitive_malloc(size_t size, void** result) {
  status_declare;
  void* a;
  a = malloc(size);
  if (a) {
    *result = a;
  } else {
    status.group = sph_helper_status_group;
    status.id = sph_helper_status_id_memory;
  };
  status_return;
}

/** like sph-helper-malloc but allocates one extra byte that is set to zero */
status_t sph_helper_primitive_malloc_string(size_t length, uint8_t** result) {
  status_declare;
  uint8_t* a;
  status_require((sph_helper_malloc((1 + length), (&a))));
  a[length] = 0;
  *result = a;
exit:
  status_return;
}
status_t sph_helper_primitive_calloc(size_t size, void** result) {
  status_declare;
  void* a;
  a = calloc(size, 1);
  if (a) {
    *result = a;
  } else {
    status.group = sph_helper_status_group;
    status.id = sph_helper_status_id_memory;
  };
  status_return;
}
status_t sph_helper_primitive_realloc(size_t size, void** memory) {
  status_declare;
  void* a;
  a = realloc((*memory), size);
  if (a) {
    *memory = a;
  } else {
    status.group = sph_helper_status_group;
    status.id = sph_helper_status_id_memory;
  };
  status_return;
}

/** display the bits of an octet */
void sph_helper_display_bits_u8(uint8_t a) {
  uint8_t i;
  printf("%u", (1 & a));
  for (i = 1; (i < 8); i = (1 + i)) {
    printf("%u", (((((uint8_t)(1)) << i) & a) ? 1 : 0));
  };
}

/** display the bits of the specified memory region */
void sph_helper_display_bits(void* a, size_t size) {
  size_t i;
  for (i = 0; (i < size); i = (1 + i)) {
    sph_helper_display_bits_u8((((uint8_t*)(a))[i]));
  };
  printf("\n");
}
