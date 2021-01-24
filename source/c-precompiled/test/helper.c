#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include "../main/sph-sp.h"
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
status_t test_helper_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, void* state) {
  status_declare;
  sp_time_t i;
  sp_channel_count_t channel_i;
  for (i = 0; (i < (end - start)); i += 1) {
    for (channel_i = 0; (channel_i < out.channels); channel_i += 1) {
      (out.samples)[channel_i][i] = ((sp_time_t)(((uint64_t)(state))));
    };
  };
  status_return;
}
sp_event_t test_helper_event(sp_time_t start, sp_time_t end, sp_time_t number) {
  sp_event_t e;
  e.start = start;
  e.end = end;
  e.generate = test_helper_event_generate;
  e.free = 0;
  e.state = ((void*)(((uint64_t)(number))));
  return (e);
}
