
#include <sph-sp/sph/futures.h>
#define sp_group_parallel_block_event(event_pointer) \
  event_pointer->prepare = sp_group_prepare; \
  event_pointer->generate = sp_group_generate_parallel_block; \
  event_pointer->config = 0
struct sp_seq_future_t;
typedef struct sp_seq_future_t sp_seq_future_t;
typedef status_t (*sp_seq_future_make)(sp_time_t, void*, void**);
typedef void (*sp_seq_future_free)(void*);
typedef status_t (*sp_seq_future_merge)(sp_time_t, sp_time_t, void*, sp_seq_future_t*, sp_size_t, void*);
typedef status_t (*sp_seq_future_run)(sp_seq_future_t*);
struct sp_seq_future_t {
  sp_time_t start;
  sp_time_t end;
  sp_time_t out_start;
  void* out;
  sp_event_t* event;
  status_t status;
  sph_future_t future;
  sp_seq_future_run run;
};
typedef struct {
  sp_seq_future_make make;
  sp_seq_future_free free;
  sp_seq_future_merge merge;
  sp_seq_future_run run;
  void* context;
} sp_seq_parallel_generic_config_t;
status_t sp_seq_parallel_generic(sp_time_t start, sp_time_t end, void* out, sp_event_list_t** events, sp_seq_parallel_generic_config_t* config);
status_t sp_seq_parallel_block(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_list_t** events);
status_t sp_group_generate_parallel_block(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* a);