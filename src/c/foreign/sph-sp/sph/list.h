
#ifndef sph_list_h_included
#define sph_list_h_included

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#define sph_slist_declare_type(name, element_type) \
  typedef struct name##_node { \
    struct name##_node* next; \
    element_type value; \
  } name##_t; \
  name##_t* name##_add_front(name##_t* head, element_type value) { \
    name##_t* node; \
    node = calloc(1, (sizeof(name##_t))); \
    if (!node) { \
      return (0); \
    }; \
    node->value = value; \
    node->next = head; \
    return (node); \
  } \
  name##_t* name##_remove_front(name##_t* head) { \
    name##_t* next; \
    if (!head) { \
      return (0); \
    }; \
    next = head->next; \
    free(head); \
    return (next); \
  } \
  void name##_destroy(name##_t* head) { \
    name##_t* next; \
    while (head) { \
      next = head->next; \
      free(head); \
      head = next; \
    }; \
  } \
  size_t name##_count(name##_t* head) { \
    size_t count = 0; \
    while (head) { \
      count += 1; \
      head = head->next; \
    }; \
    return (count); \
  } \
  void name##_append(name##_t* head, name##_t* tail) { \
    if (head) { \
      head->next = tail; \
    }; \
  }
#define sph_dlist_declare_type(name, element_type) \
  typedef struct name##_node { \
    struct name##_node* previous; \
    struct name##_node* next; \
    element_type value; \
  } name##_t; \
  void name##_reverse(name##_t** head) { \
    name##_t* node; \
    name##_t* next; \
    name##_t* last; \
    if (!head) { \
      return; \
    }; \
    node = *head; \
    last = 0; \
    while (node) { \
      next = node->next; \
      node->next = node->previous; \
      node->previous = next; \
      last = node; \
      node = next; \
    }; \
    *head = last; \
  } \
  void name##_validate(name##_t* head) { \
    size_t index; \
    name##_t* node; \
    name##_t* previous; \
    index = 0; \
    previous = 0; \
    node = head; \
    while (node) { \
      if (!(previous == node->previous)) { \
        printf("invalid previous link at index %zu, node %p\n", index, ((void*)(node))); \
        return; \
      }; \
      if ((node->next == node->previous) && !node->next) { \
        printf("circular link at index %zu, node %p\n", index, ((void*)(node))); \
        return; \
      }; \
      previous = node; \
      node = node->next; \
      index += 1; \
    }; \
  } \
  void name##_unlink(name##_t** head, name##_t* node) { \
    if (!node) { \
      return; \
    }; \
    if (node->previous) { \
      node->previous->next = node->next; \
    } else { \
      if (head) { \
        *head = node->next; \
      }; \
    }; \
    if (node->next) { \
      node->next->previous = node->previous; \
    }; \
  } \
  void name##_print(name##_t* head) { \
    name##_t* node = head; \
    while (node) { \
      printf(("%p <- %p -> %p\n"), ((void*)(node->previous)), ((void*)(node)), ((void*)(node->next))); \
      node = node->next; \
    }; \
  }
#endif
