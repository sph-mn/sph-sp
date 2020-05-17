#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include "../main/sph-sp.h"
#include <sph/float.c>
#include <sph/helper.c>
#include <sph/memreg.c>
#include <sph/string.c>
#include <sph/filesystem.c>
#define test_helper_test_one(func) \
  printf("%s\n", #func); \
  status_require((func()))
#define test_helper_assert(description, expression) \
  if (!expression) { \
    printf("%s failed\n", description); \
    status_set_goto("sph-sp", 1); \
  }
#define test_helper_display_summary() \
  if (status_is_success) { \
    printf(("--\ntests finished successfully.\n")); \
  } else { \
    printf(("\ntests failed. %d %s\n"), (status.id), (sp_status_description(status))); \
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
void test_helper_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_time_t i;
  sp_channels_t channel_i;
  for (i = 0; (i < (end - start)); i += 1) {
    for (channel_i = 0; (channel_i < out.channels); channel_i += 1) {
      (out.samples)[channel_i][i] = ((sp_time_t)(event->state));
    };
  };
}
sp_event_t test_helper_event(sp_time_t start, sp_time_t end, sp_time_t number) {
  sp_event_t e;
  e.start = start;
  e.end = end;
  e.f = test_helper_event_f;
  e.free = 0;
  e.state = ((void*)(number));
  return (e);
}
