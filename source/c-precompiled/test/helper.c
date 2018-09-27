#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include "../foreign/sph/helper.c"
#include "../foreign/sph/memreg.c"
#include "../foreign/sph/string.c"
#include "../foreign/sph/filesystem.c"
#define test_helper_test_one(func) \
  printf("%s\n", #func); \
  status_require((func()))
#define test_helper_assert(description, expression) \
  if (!expression) { \
    printf("%s failed\n", description); \
    status_set_id_goto(1); \
  }
#define test_helper_display_summary() \
  if (status_is_success) { \
    printf(("--\ntests finished successfully.\n")); \
  } else { \
    printf(("\ntests failed. %d %s\n"), (status.id), (sp_status_description(status))); \
  }
void debug_log_samples(sp_sample_t* a, size_t len) {
  size_t column_width;
  size_t column_end;
  size_t index;
  column_width = 8;
  index = 0;
  while ((index < len)) {
    column_end = (index + column_width);
    while (((index < len) && (index < column_end))) {
      printf("%f ", (a[index]));
      index = (1 + index);
    };
    printf("\n");
  };
};