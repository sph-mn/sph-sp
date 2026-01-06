
#ifndef sph_array_h_included
#define sph_array_h_included

/* depends on stdlib.h (malloc/realloc/free) for the default allocators */
#include <sph-sp/sph/status.h>

#define sph_array_status_id_memory 1
#define sph_array_status_group "sph"
#define sph_array_growth_factor 2
#define sph_array_memory_error status_set_goto(sph_array_status_group, sph_array_status_id_memory)
#define sph_array_default_alloc(s, es) malloc((s * es))
#define sph_array_default_realloc(d, s, u, n, es) realloc(d, (n * es))
#define sph_array_declare_no_struct_type(name, element_type)
#define sph_array_default_declare_struct_type(name, element_type, size_type) \
  typedef struct { \
    size_type size; \
    size_type used; \
    element_type* data; \
  } name##_t
#define sph_array_declare_type_custom(name, element_type, sph_array_alloc, sph_array_realloc, sph_array_free, sph_array_declare_struct_type, size_type, size_max) \
  sph_array_declare_struct_type(name, element_type, size_type); \
  status_t name##_init(size_type size, name##_t* a) { \
    status_declare; \
    element_type* data = sph_array_alloc(size, (sizeof(element_type))); \
    if (!data) { \
      sph_array_memory_error; \
    }; \
    a->data = data; \
    a->size = size; \
    a->used = 0; \
  exit: \
    status_return; \
  } \
  status_t name##_resize(name##_t* a, size_type new_size) { \
    status_declare; \
    element_type* data = sph_array_realloc((a->data), (a->size), (a->used), new_size, (sizeof(element_type))); \
    if (!data) { \
      sph_array_memory_error; \
    }; \
    a->data = data; \
    a->size = new_size; \
    a->used = ((new_size < a->used) ? new_size : a->used); \
  exit: \
    status_return; \
  } \
  void name##_uninit(name##_t* a) { sph_array_free((a->data)); } \
  status_t name##_ensure(name##_t* a, size_type needed) { \
    status_declare; \
    size_type old_size; \
    size_type new_size; \
    element_type* data; \
    old_size = a->size; \
    if (old_size >= (a->used + needed)) { \
      goto exit; \
    }; \
    if (old_size == size_max) { \
      sph_array_memory_error; \
    }; \
    new_size = old_size; \
    do { \
      if (new_size < (size_max / sph_array_growth_factor)) { \
        new_size *= sph_array_growth_factor; \
      } else { \
        new_size = size_max; \
        break; \
      }; \
    } while ((new_size < (old_size + needed))); \
    data = sph_array_realloc((a->data), (a->size), (a->used), new_size, (sizeof(element_type))); \
    if (!data) { \
      sph_array_memory_error; \
    }; \
    a->data = data; \
    a->size = new_size; \
  exit: \
    status_return; \
  }
#define sph_array_declare_type(name, element_type) sph_array_declare_type_custom(name, element_type, sph_array_default_alloc, sph_array_default_realloc, free, sph_array_default_declare_struct_type, size_t, SIZE_MAX)
#define sph_array_declare(a, type) type a = { 0 }
#define sph_array_add(a, value) \
  (a.data)[a.used] = value; \
  a.used += 1
#define sph_array_set_null(a) \
  a.used = 0; \
  a.size = 0; \
  a.data = 0
#define sph_array_get(a, index) (a.data)[index]
#define sph_array_get_pointer(a, index) (a.data + index)
#define sph_array_clear(a) a.used = 0
#define sph_array_remove(a) a.used -= 1
#define sph_array_remove_swap(a, i) \
  if ((1 + i) < a.used) { \
    (a.data)[i] = (a.data)[a.used]; \
  }; \
  a.used -= 1
#define sph_array_unused_size(a) (a.size - a.used)
#define sph_array_full(a) (a.used == a.size)
#define sph_array_not_full(a) (a.used < a.size)
#define sph_array_take(a, data, size, used) \
  a->data = data; \
  a->size = size; \
  a->used = used
#define sph_array_last(a) (a.data)[(a.used - 1)]
#define sph_array_unused(a) (a.data)[a.used]
#define sph_array_unused_pointer(a) (a.data + a.used)
#define sph_array_declare_stack(name, array_size, type_t, value_t) \
  value_t name##_data[array_size]; \
  type_t name; \
  name.data = name##_data; \
  name.size = array_size; \
  name.used = 0
#endif
