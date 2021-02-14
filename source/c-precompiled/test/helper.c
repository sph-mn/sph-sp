
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include "../main/sph-sp.h"
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
status_t test_helper_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_time_t i;
  sp_channel_count_t ci;
  uint64_t value;
  value = ((sp_time_t)(((uint64_t)(event->state))));
  for (i = start; (i < end); i += 1) {
    for (ci = 0; (ci < out.channels); ci += 1) {
      (out.samples)[ci][(i - start)] = value;
    };
  };
  status_return;
}
sp_event_t test_helper_event(sp_time_t start, sp_time_t end, sp_time_t number) {
  sp_declare_event(e);
  e.start = start;
  e.end = end;
  e.generate = test_helper_event_generate;
  e.free = 0;
  e.state = ((void*)(((uint64_t)(number))));
  return (e);
}
