
#define sp_event_reset(x) x = sp_event_null
#define sp_declare_event(id) \
  sp_event_t id = { 0 }; \
  id.memory.data = 0
#define sp_declare_event_2(id1, id2) \
  sp_declare_event(id1); \
  sp_declare_event(id2)
#define sp_declare_event_3(id1, id2, id3) \
  sp_declare_event(id1); \
  sp_declare_event(id2); \
  sp_declare_event(id3)
#define sp_declare_event_4(id1, id2, id3, id4) \
  sp_declare_event_2(id1, id2); \
  sp_declare_event_2(id3, id4)
#define sp_declare_group(id) \
  sp_declare_event(id); \
  id.prepare = sp_group_prepare
#define sp_declare_group_parallel(id) \
  sp_declare_event(id); \
  id.prepare = sp_group_prepare_parallel
#define sp_declare_event_list(id) sp_event_list_t* id = 0
#define sp_event_duration(a) (a.end - a.start)
#define sp_event_duration_set(a, duration) a.end = (a.start + duration)
#define sp_event_move(a, start) \
  a.end = (start + (a.end - a.start)); \
  a.start = start
#define sp_group_size_t uint16_t
#define sp_event_memory_add(event, address) sp_event_memory_add_with_handler(event, address, free)
#define sp_event_memory_fixed_add(event, address) sp_event_memory_fixed_add_with_handler(event, address, free)
#define sp_sine_config_t sp_wave_event_config_t
#define sp_memory_add array3_add
#define sp_seq_events_prepare sp_event_list_reverse
#define free_event_on_error(event_address) free_on_error((event_address->free), event_address)
#define free_event_on_exit(event_address) free_on_exit((event_address->free), event_address)
#define sp_group_event_list(event) ((sp_event_list_t**)(&(event->data)))
#define sp_event_free(a) \
  if (a.free) { \
    (a.free)((&a)); \
  }
#define sp_event_pointer_free(a) \
  if (a->free) { \
    (a->free)(a); \
  }

/** use case: event variables defined at the top-level */
#define sp_define_event(name, _prepare, duration) sp_event_t name = { .prepare = _prepare, .start = 0, .end = duration, .data = 0, .memory = { 0 } }

/** allocated memory with malloc, save address in pointer at pointer-address,
     and also immediately add the memory to event memory to be freed with event.free */
#define sp_event_memory_malloc(event, count, type, pointer_address) \
  sp_malloc_type(count, type, pointer_address); \
  sp_event_memory_add(_event, (*pointer_address))
#define sp_event_config_load(variable_name, type, event) type variable_name = *((type*)(event->config))
#define sp_sound_event(event_pointer, _config) \
  event_pointer->prepare = sp_sound_event_prepare; \
  event_pointer->config = _config
#define sp_wave_event(event_pointer, _config) \
  event_pointer->prepare = sp_wave_event_prepare; \
  event_pointer->config = _config
#define sp_noise_event(event_pointer, _config) \
  event_pointer->prepare = sp_noise_event_prepare; \
  event_pointer->config = _config
#define sp_cheap_noise_event(event_pointer, _config) \
  event_pointer->prepare = sp_cheap_noise_event_prepare; \
  event_pointer->config = _config
#define sp_group_event(event_pointer) event_pointer->prepare = sp_group_prepare
#define sp_event_prepare(a) \
  if (a.prepare) { \
    status_require(((a.prepare)((&a)))); \
    a.prepare = 0; \
  }
#define sp_event_alloc(event_pointer, allocator, pointer_address) \
  status_require((allocator(pointer_address))); \
  sp_event_memory_add(event_pointer, (*pointer_address))
#define sp_event_alloc1(event_pointer, allocator, size, pointer_address) \
  status_require((allocator(size, pointer_address))); \
  sp_event_memory_add(event_pointer, (*pointer_address))
#define sp_event_malloc(event_pointer, size, pointer_address) \
  status_require((sph_helper_malloc(size, pointer_address))); \
  sp_event_memory_add(event_pointer, (*pointer_address))
#define sp_event_malloc_type_n *(event_pointer, count, type, pointer_address)sp_event_malloc(event_pointer, (count * sizeof(type)), pointer_address)
#define sp_event_malloc_type(event_pointer, type, pointer_address) sp_event_malloc(event_pointer, (sizeof(type)), pointer_address)
#define sp_event_samples(event_pointer, size, pointer_address) sp_event_alloc1(event_pointer, sp_samples_new, size, pointer_address)
#define sp_event_times(event_pointer, size, pointer_address) sp_event_alloc(event_pointer, sp_times_new, size, pointer_address)
#define sp_event_units(event_pointer, size, pointer_address) sp_event_alloc(event_pointer, sp_units_new, size, pointer_address)
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
#define sp_event_path_samples_c3_srq(event_pointer, out, ...) \
  status_require((sp_path_samples_c3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples_c4_srq(event_pointer, out, ...) \
  status_require((sp_path_samples_c4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_samples_c5_srq(event_pointer, out, ...) \
  status_require((sp_path_samples_c5(out, __VA_ARGS__))); \
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
#define sp_event_path_times_c3_srq(event_pointer, out, ...) \
  status_require((sp_path_times_c3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times_c4_srq(event_pointer, out, ...) \
  status_require((sp_path_times_c4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_path_times_c5_srq(event_pointer, out, ...) \
  status_require((sp_path_times_c5(out, __VA_ARGS__))); \
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
#define sp_event_envelope_zero_c3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero_c3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero_c4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero_c4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_zero_c5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_zero_c5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled_c3_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled_c3(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled_c4_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled_c4(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
#define sp_event_envelope_scaled_c5_srq(event_pointer, out, ...) \
  status_require((sp_envelope_scaled_c5(out, __VA_ARGS__))); \
  status_require((sp_event_memory_add(event_pointer, (*out))))
array3_declare_type(sp_memory, memreg2_t);
typedef void (*sp_memory_free_t)(void*);
struct sp_event_t;
typedef struct sp_event_t {
  sp_time_t start;
  sp_time_t end;
  status_t (*generate)(sp_time_t, sp_time_t, sp_block_t, struct sp_event_t*);
  status_t (*prepare)(struct sp_event_t*);
  void (*free)(struct sp_event_t*);
  void* data;
  void* config;
  sp_memory_t memory;
} sp_event_t;
typedef status_t (*sp_event_generate_t)(sp_time_t, sp_time_t, sp_block_t, sp_event_t*);
typedef struct sp_event_list_struct {
  struct sp_event_list_struct* previous;
  struct sp_event_list_struct* next;
  sp_event_t event;
} sp_event_list_t;
typedef struct {
  sp_bool_t use;
  sp_bool_t mute;
  sp_time_t delay;
  sp_time_t phs;
  sp_sample_t amp;
  sp_sample_t* amod;
} sp_channel_config_t;
typedef struct {
  sp_sample_t* wvf;
  sp_time_t wvf_size;
  sp_time_t phs;
  sp_time_t frq;
  sp_time_t* fmod;
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_channel_count_t channel_count;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_wave_event_config_t;
typedef struct {
  sp_sample_t* wvf;
  sp_time_t wvf_size;
  sp_time_t phs;
  sp_time_t frq;
  sp_time_t* fmod;
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_channel_count_t channel;
} sp_wave_event_state_t;
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_sample_t cutl;
  sp_sample_t cuth;
  sp_sample_t trnl;
  sp_sample_t trnh;
  sp_sample_t* cutl_mod;
  sp_sample_t* cuth_mod;
  sp_time_t resolution;
  uint8_t is_reject;
  sp_channel_count_t channel_count;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_noise_event_config_t;
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_sample_t cut;
  sp_sample_t* cut_mod;
  sp_sample_t q_factor;
  sp_sample_t* q_factor_mod;
  sp_time_t passes;
  sp_state_variable_filter_t type;
  sp_random_state_t* random_state;
  sp_time_t resolution;
  sp_channel_count_t channel_count;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_cheap_noise_event_config_t;
typedef struct {
  sp_bool_t noise;
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_time_t frq;
  sp_time_t* fmod;
  sp_time_t phs;
  sp_time_t wdt;
  sp_time_t* wmod;
  sp_channel_count_t channel_count;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_sound_event_config_t;
status_t (*sp_event_prepare_t)(sp_event_t*);
typedef status_t (*sp_map_generate_t)(sp_time_t, sp_time_t, sp_block_t, sp_block_t, void*);
typedef struct {
  sp_event_t event;
  sp_map_generate_t map_generate;
  void* state;
} sp_map_event_state_t;
typedef struct {
  sp_event_t event;
  sp_map_generate_t map_generate;
  void* state;
  sp_bool_t isolate;
} sp_map_event_config_t;
void sp_channel_config_zero(sp_channel_config_t* a);
sp_event_t sp_event_null = { 0 };
void sp_event_list_display(sp_event_list_t* a);
void sp_event_list_reverse(sp_event_list_t** a);
void sp_event_list_validate(sp_event_list_t* a);
void sp_event_list_remove_element(sp_event_list_t** a, sp_event_list_t* element);
status_t sp_event_list_add(sp_event_list_t** a, sp_event_t event);
void sp_event_list_free(sp_event_list_t** events);
status_t sp_event_memory_ensure(sp_event_t* a, sp_time_t additional_size);
status_t sp_event_memory_add_with_handler(sp_event_t* event, void* address, sp_memory_free_t handler);
void sp_event_memory_fixed_add_with_handler(sp_event_t* event, void* address, sp_memory_free_t handler);
void sp_event_memory_free(sp_event_t* event);
status_t sp_seq(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_list_t** events);
status_t sp_seq_parallel(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_list_t** events);
status_t sp_wave_event_prepare(sp_event_t* event);
status_t sp_noise_event_prepare(sp_event_t* event);
status_t sp_cheap_noise_event_prepare(sp_event_t* event);
status_t sp_group_prepare(sp_event_t* event);
status_t sp_group_prepare_parallel(sp_event_t* a);
status_t sp_group_add(sp_event_t* a, sp_event_t event);
status_t sp_group_append(sp_event_t* a, sp_event_t event);
status_t sp_group_add_set(sp_event_t* group, sp_time_t start, sp_time_t duration, sp_event_t event);
void sp_group_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event);
void sp_group_event_parallel_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event);
void sp_group_event_free(sp_event_t* a);
status_t sp_map_event_prepare(sp_event_t* event);
sp_channel_config_t sp_channel_config(sp_bool_t mute, sp_time_t delay, sp_time_t phs, sp_sample_t amp, sp_sample_t* amod);
void sp_group_free();
void sp_wave_event_free();
void sp_noise_event_free();
void sp_cheap_noise_event_free();
void sp_map_event_free();
status_t sp_noise_event_config_new(sp_noise_event_config_t** out);
status_t sp_cheap_noise_event_config_new(sp_cheap_noise_event_config_t** out);
status_t sp_wave_event_config_new(sp_wave_event_config_t** out);
status_t sp_map_event_config_new(sp_map_event_config_t** out);
void sp_wave_event_config_defaults(sp_wave_event_config_t* config);
status_t sp_sound_event_prepare(sp_event_t* event);
status_t sp_sound_event_config_new(sp_sound_event_config_t** out);