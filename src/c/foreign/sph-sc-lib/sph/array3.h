
/* "array3" - dynamic array as a struct {.data, .size, .used} that combines memory pointer, length and used length in one object.
   the "used" property is to support variable length data in a fixed size memory area.
   depends on stdlib.h for malloc. custom allocators can be used.
   examples:
     array3_declare_type(my_type, int);
     my_type_t a;
     if(my_type_new(4, &a)) {
       // memory allocation error
     }
     array3_add(a, 1);
     array3_add(a, 2);
     size_t i = 0;
     for(i = 0; i < a.size; i += 1) {
       array3_get(a, i);
     }
     array3_free(a); */

#define array3_declare_type(name, element_type) array3_declare_type_custom(name, element_type, malloc, realloc)
#define array3_declare_type_custom(name, element_type, malloc, realloc) \
  typedef struct { \
    element_type* data; \
    size_t size; \
    size_t used; \
  } name##_t; \
  /** return 0 on success, 1 for memory allocation error */ \
  uint8_t name##_new(size_t size, name##_t* a) { \
    element_type* data; \
    data = malloc((size * sizeof(element_type))); \
    if (!data) { \
      return (1); \
    }; \
    a->data = data; \
    a->size = size; \
    a->used = 0; \
    return (0); \
  } \
\
  /** return 0 on success, 1 for realloc error */ \
  uint8_t name##_resize(name##_t* a, size_t new_size) { \
    element_type* data = realloc((a->data), (new_size * sizeof(element_type))); \
    if (!data) { \
      return (1); \
    }; \
    a->data = data; \
    a->size = new_size; \
    a->used = ((new_size < a->used) ? new_size : a->used); \
    return (0); \
  }
#define array3_declare(a, type) type a = { 0, 0, 0 }
#define array3_add(a, value) \
  (a.data)[a.used] = value; \
  a.used += 1
#define array3_set_null(a) \
  a.used = 0; \
  a.size = 0; \
  a.data = 0
#define array3_get(a, index) (a.data)[index]
#define array3_clear(a) a.used = 0
#define array3_remove(a) a.used -= 1
#define array3_size(a) a.used
#define array3_unused_size(a) (a.size - a.used)
#define array3_max_size(a) a.size
#define array3_free(a) free((a.data))
#define array3_full(a) (a.used == a.size)
#define array3_not_full(a) (a.used < a.size)
#define array3_take(a, data, size, used) \
  a->data = data; \
  a->size = size; \
  a->used = used
#define array3_data_last(a) (a.data)[(a.size - 1)]
#define array3_declare_stack(name, array_size, type_t, value_t) \
  value_t name##_data[array_size]; \
  type_t name; \
  name.data = name##_data; \
  name.size = array_size
