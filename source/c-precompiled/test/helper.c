#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include "../main/sph-sp.h"
#include "../foreign/sph/float.c"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/memreg.c"
#include "../foreign/sph/string.c"
#include "../foreign/sph/filesystem.c"
#define test_helper_test_one(func) \
  printf("%s\n", #func); \
  s((func()))
#define test_helper_assert(description, expression) \
  if (!expression) { \
    printf("%s failed\n", description); \
    s_set_goto("sph-sp", 1); \
  }
#define test_helper_display_summary() \
  if (s_is_success) { \
    printf(("--\ntests finished successfully.\n")); \
  } else { \
    printf(("\ntests failed. %d %s\n"), (s_current.id), (sp_status_description(s_current))); \
  }
/** display a sample array in one line */
void debug_display_sample_array(sp_sample_t* a, sp_time_t len) {
  sp_time_t i;
  printf(("%.17g"), (a[0]));
  for (i = 1; (i < len); i = (1 + i)) {
    printf((" %.17g"), (a[i]));
  };
  printf("\n");
}
