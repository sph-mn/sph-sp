
#define sp_seq_events_prepare sp_event_list_reverse
#define sp_default_resolution ((sp_rate < 10000) ? sp_rate : (sp_rate / 1000))
#define sp_event_reset(x) x = sp_null_event
#define sp_declare_event(id) \
  sp_event_t id = { 0 }; \
  id.memory.data = 0
#define sp_declare_event_list(id) sp_event_list_t* id = 0
#define sp_event_duration(a) (a.end - a.start)
#define sp_group_event_list(event) ((sp_event_list_t**)(&(event->config)))
#define sp_event_free(a) \
  if (a.free) { \
    (a.free)((&a)); \
  }
#define sp_event_prepare_srq(a) \
  status_require(((a.prepare)((&a)))); \
  a.prepare = 0
#define sp_event_prepare_optional_srq(a) \
  if (a.prepare) { \
    sp_event_prepare_srq(a); \
  }
#define sp_event_memory_array_add array3_add
#define sp_event_memory_add(event, address) sp_event_memory_add_with_handler(event, address, free)
#define sp_event_memory_fixed_add(event, address) sp_event_memory_fixed_add_with_handler(event, address, free)
#define sp_wave_event_config_new(out) sp_wave_event_config_new_n(1, out)
#define sp_map_event_config_new(out) sp_map_event_config_new_n(1, out)
#define sp_noise_event_config_new(out) sp_noise_event_config_new_n(1, out)

/** use case: event variables defined at the top-level */
#define sp_define_event(name, _prepare, duration) sp_event_t name = { .prepare = _prepare, .start = 0, .end = duration, .config = 0, .memory = { 0 } }
#define sp_wave_event(event_pointer, _config) \
  event_pointer->prepare = sp_wave_event_prepare; \
  event_pointer->generate = sp_wave_event_generate; \
  event_pointer->config = _config
#define sp_noise_event(event_pointer, _config) \
  event_pointer->prepare = sp_noise_event_prepare; \
  event_pointer->generate = sp_noise_event_generate; \
  event_pointer->config = _config
#define sp_map_event(event_pointer, _config) \
  event_pointer->prepare = sp_map_event_prepare; \
  event_pointer->generate = (_config->isolate ? sp_map_event_isolated_generate : sp_map_event_generate); \
  event_pointer->config = _config
#define sp_group_event(event_pointer) \
  event_pointer->prepare = sp_group_prepare; \
  event_pointer->generate = sp_group_generate; \
  event_pointer->config = 0
#define sp_event_path_samples_srq(event_pointer, out, ...) \
  status_require((sp_path_samples(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples3_srq(event_pointer, out, ...) \
  status_require((sp_path_samples3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples4_srq(event_pointer, out, ...) \
  status_require((sp_path_samples4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples5_srq(event_pointer, out, ...) \
  status_require((sp_path_samples5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples_curve3_srq(event_pointer, out, ...) \
  status_require((sp_path_samples_curve3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples_curve4_srq(event_pointer, out, ...) \
  status_require((sp_path_samples_curve4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples_curve5_srq(event_pointer, out, ...) \
  status_require((sp_path_samples_curve5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times_srq(event_pointer, out, ...) \
  status_require((sp_path_times(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times3_srq(event_pointer, out, ...) \
  status_require((sp_path_times3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times4_srq(event_pointer, out, ...) \
  status_require((sp_path_times4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times5_srq(event_pointer, out, ...) \
  status_require((sp_path_times5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times_curve3_srq(event_pointer, out, ...) \
  status_require((sp_path_times_curve3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times_curve4_srq(event_pointer, out, ...) \
  status_require((sp_path_times_curve4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times_curve5_srq(event_pointer, out, ...) \
  status_require((sp_path_times_curve5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero_curve3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero_curve3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero_curve4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero_curve4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero_curve5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero_curve5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale_curve3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale_curve3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale_curve4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale_curve4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scale_curve5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scale_curve5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_alloc_srq(event_pointer, allocator, size, pointer_address) \
  status_require((allocator(size, pointer_address))); \
  status_require((sp_event_memory_add(event_pointer, (*pointer_address))))
#define sp_event_malloc_srq(event_pointer, size, pointer_address) \
  status_require((sph_helper_malloc(size, pointer_address))); \
  status_require((sp_event_memory_add(event_pointer, (*pointer_address))))
#define sp_event_malloc_type_n_srq(event_pointer, count, type, pointer_address) sp_event_malloc_srq(event_pointer, (count * sizeof(type)), pointer_address)
#define sp_event_malloc_type_srq(event_pointer, type, pointer_address) sp_event_malloc_type_n_srq(event_pointer, 1, type, pointer_address)
#define sp_event_samples_srq(event_pointer, size, pointer_address) sp_event_alloc_srq(event_pointer, sp_samples_new, size, pointer_address)
#define sp_event_times_srq(event_pointer, size, pointer_address) sp_event_alloc_srq(event_pointer, sp_times_new, size, pointer_address)
#define sp_event_units_srq(event_pointer, size, pointer_address) sp_event_alloc_srq(event_pointer, sp_units_new, size, pointer_address)
#define sp_event_config_get(a, type) *((type*)(a.config))
array3_declare_type(sp_memory, memreg2_t);
typedef void (*sp_memory_free_t)(void*);
struct sp_event_t;
typedef struct sp_event_t {
  sp_time_t start;
  sp_time_t end;
  status_t (*generate)(sp_time_t, sp_time_t, void*, struct sp_event_t*);
  status_t (*prepare)(struct sp_event_t*);
  status_t (*update)(struct sp_event_t*);
  void (*free)(struct sp_event_t*);
  void* config;
  sp_memory_t memory;
} sp_event_t;
typedef status_t (*sp_event_generate_t)(sp_time_t, sp_time_t, void*, sp_event_t*);
typedef status_t (*sp_event_prepare_t)(sp_event_t*);
typedef status_t (*sp_map_generate_t)(sp_time_t, sp_time_t, void*, void*, void*);
typedef struct sp_event_list_struct {
  struct sp_event_list_struct* previous;
  struct sp_event_list_struct* next;
  sp_event_t event;
} sp_event_list_t;
typedef status_t (*sp_event_block_generate_t)(sp_time_t, sp_time_t, sp_time_t, void*, sp_event_t*);
typedef struct {
  void* config;
  sp_event_t event;
  sp_map_generate_t map_generate;
  sp_bool_t isolate;
} sp_map_event_config_t;
typedef struct {
  sp_sample_t* amod;
  sp_sample_t amp;
  sp_channel_count_t channel;
  sp_time_t* fmod;
  sp_frq_t frq;
  sp_time_t phs;
  sp_time_t* pmod;
  sp_bool_t use;
} sp_wave_event_channel_config_t;
typedef struct {
  sp_sample_t* wvf;
  sp_time_t wvf_size;
  sp_channel_count_t channel_count;
  sp_wave_event_channel_config_t channel_config[sp_channel_count_limit];
} sp_wave_event_config_t;
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_channel_count_t channel;
  sp_convolution_filter_state_t* filter_state;
  sp_frq_t frq;
  sp_time_t* fmod;
  sp_frq_t wdt;
  sp_time_t* wmod;
  sp_frq_t trnl;
  sp_frq_t trnh;
  sp_bool_t use;
} sp_noise_event_channel_config_t;
typedef struct {
  sp_bool_t is_reject;
  sp_random_state_t random_state;
  sp_time_t resolution;
  sp_sample_t* temp[3];
  sp_channel_count_t channel_count;
  sp_noise_event_channel_config_t channel_config[sp_channel_count_limit];
} sp_noise_event_config_t;
sp_event_t sp_null_event = { 0 };
status_t sp_event_list_add(sp_event_list_t** a, sp_event_t event);
void sp_event_list_display(sp_event_list_t* a);
void sp_event_list_free(sp_event_list_t** events);
void sp_event_list_remove(sp_event_list_t** a, sp_event_list_t* element);
void sp_event_list_reverse(sp_event_list_t** a);
void sp_event_list_validate(sp_event_list_t* a);
status_t sp_event_memory_add_with_handler(sp_event_t* event, void* address, sp_memory_free_t handler);
status_t sp_event_memory_ensure(sp_event_t* a, sp_time_t additional_size);
void sp_event_memory_fixed_add_with_handler(sp_event_t* event, void* address, sp_memory_free_t handler);
void sp_event_memory_free(sp_event_t* event);
status_t sp_group_add(sp_event_t* a, sp_event_t event);
status_t sp_group_append(sp_event_t* a, sp_event_t event);
void sp_group_event_free(sp_event_t* a);
void sp_group_event_f(sp_time_t start, sp_time_t end, void* out, sp_event_t* event);
void sp_group_free(sp_event_t* group);
status_t sp_group_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* a);
status_t sp_group_prepare(sp_event_t* event);
status_t sp_map_event_config_new_n(sp_time_t count, sp_map_event_config_t** out);
void sp_map_event_free(sp_event_t* event);
status_t sp_map_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event);
status_t sp_map_event_isolated_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event);
status_t sp_map_event_prepare(sp_event_t* event);
sp_noise_event_config_t sp_noise_event_config_defaults();
status_t sp_noise_event_config_new_n(sp_time_t count, sp_noise_event_config_t** out);
void sp_noise_event_free(sp_event_t* event);
status_t sp_noise_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event);
status_t sp_noise_event_prepare(sp_event_t* event);
status_t sp_seq(sp_time_t start, sp_time_t end, void* out, sp_event_list_t** events);
sp_wave_event_config_t sp_wave_event_config_defaults();
status_t sp_wave_event_config_new_n(sp_time_t count, sp_wave_event_config_t** out);
status_t sp_wave_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event);
status_t sp_wave_event_prepare(sp_event_t* event);
sp_event_t sp_event_schedule(sp_event_t event, sp_time_t onset, sp_time_t duration, void* config);