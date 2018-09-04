#include <byteswap.h>
#include <math.h>
#include "./foreign/sph.c"
#include "./foreign/sph/status.c"
#include "./foreign/sph/float.c"
#include "./main/config.c"
#define sp_port_type_alsa 0
#define sp_port_type_file 1
#define sp_port_bit_input 1
#define sp_port_bit_output 2
#define sp_port_bit_position 4
#define sp_port_bit_closed 8
#define sp_sample_type_f64 1
#define sp_sample_type_f32 2
#define sp_octets_to_samples(a) (a/sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a*sizeof(sp_sample_t))
#define duration_to_sample_count(seconds,sample_rate) (seconds*sample_rate)
#define sample_count_to_duration(sample_count,sample_rate) (sample_count/sample_rate)
/** set sph/status object to group sp and id memory-error and goto "exit" label if "a" is 0 */
#define sp_alloc_require(a) if(!a){status_set_both_goto(sp_status_group_sp,sp_status_id_memory);}
#define sp_alloc_set(a,octet_count) a=malloc(octet_count);sp_alloc_require(a)
#define sp_alloc_set_zero(a,octet_count) a=calloc(octet_count,1);sp_alloc_require(a)
#define sp_alloc_define(id,type,octet_count) type id;sp_alloc_set(id,octet_count)
#define sp_alloc_define_zero(id,type,octet_count) type id;sp_alloc_set_zero(id,octet_count)
#define sp_alloc_define_samples(id,sample_count) sp_alloc_define(id,sp_sample_t*,(sample_count*sizeof(sp_sample_t)))
#define sp_alloc_set_samples(a,sample_count) sp_alloc_set(a,(sample_count*sizeof(sp_sample_t)))
#define sp_alloc_define_samples_zero(id,sample_count) sp_alloc_define_zero(id,sp_sample_t*,(sp_samples_to_octets((sizeof(sp_sample_t)))))
#define sp_alloc_set_samples_zero(a,sample_count) sp_alloc_set_zero(a,(sp_samples_to_octets(sample_count)))
#define kiss_fft_scalar sp_sample_t
/** sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value */
#define sp_windowed_sinc_cutoff(freq,sample_rate) ((2*M_PI*freq)/sample_rate)
#if (sp_sample_type==sp_sample_type_f64)
#define sp_sample_t f64_s
#define sample_reverse_endian __bswap_64
#define sp_sample_sum f64_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT64_LE
#else
#if (sp_sample_type_f32==sp_sample_type)
#define sp_sample_t f32_s
#define sample_reverse_endian __bswap_32
#define sp_sample_sum f32_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT_LE
#endif
#endif
typedef struct{uint8_t type;uint8_t flags;uint32_t sample_rate;uint32_t channel_count;void* data;} sp_port_t;status_t sp_port_read(sp_sample_t** result,sp_port_t* port,uint32_t sample_count);status_t sp_port_write(sp_port_t* port,size_t sample_count,sp_sample_t** channel_data);status_t sp_port_position(size_t* result,sp_port_t* port);status_t sp_port_set_position(sp_port_t* port,size_t sample_index);status_t sp_file_open(sp_port_t* result,uint8_t* path,uint32_t channel_count,uint32_t sample_rate);status_t sp_alsa_open(sp_port_t* result,uint8_t* device_name,boolean input_p,uint32_t channel_count,uint32_t sample_rate,int32_t latency);status_t sp_port_close(sp_port_t* a);
#include <generic/base/io.h>
sp_sample_t** sp_alloc_channel_array(uint32_t channel_count,uint32_t sample_count);uint8_t* sp_status_description(status_t a);uint8_t* sp_status_name(status_t a);sp_float_t sp_sin_lq(sp_float_t a);sp_float_t sp_sinc(sp_float_t a);void sp_sine(sp_sample_t* data,uint32_t len,sp_float_t sample_duration,sp_float_t freq,sp_float_t phase,sp_float_t amp);void sp_sine_lq(sp_sample_t* data,uint32_t len,sp_float_t sample_duration,sp_float_t freq,sp_float_t phase,sp_float_t amp);typedef struct{sp_sample_t* data;size_t data_len;size_t ir_len_prev;sp_sample_t* ir;size_t ir_len;uint32_t sample_rate;sp_float_t freq;sp_float_t transition;} sp_windowed_sinc_state_t;size_t sp_windowed_sinc_ir_length(sp_float_t transition);void sp_windowed_sinc_ir(sp_sample_t** result,size_t* result_len,uint32_t sample_rate,sp_float_t freq,sp_float_t transition);void sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t* state);uint8_t sp_windowed_sinc_state_create(uint32_t sample_rate,sp_float_t freq,sp_float_t transition,sp_windowed_sinc_state_t** state);status_i_t sp_windowed_sinc(sp_sample_t* result,sp_sample_t* source,size_t source_len,uint32_t sample_rate,sp_float_t freq,sp_float_t transition,sp_windowed_sinc_state_t** state);sp_float_t sp_window_blackman(sp_float_t a,size_t width);void sp_spectral_inversion_ir(sp_sample_t* a,size_t a_len);void sp_spectral_reversal_ir(sp_sample_t* a,size_t a_len);status_t sp_fft(sp_sample_t* result,uint32_t result_len,sp_sample_t* source,uint32_t source_len);status_t sp_ifft(sp_sample_t* result,uint32_t result_len,sp_sample_t* source,uint32_t source_len);status_i_t sp_moving_average(sp_sample_t* result,sp_sample_t* source,uint32_t source_len,sp_sample_t* prev,uint32_t prev_len,sp_sample_t* next,uint32_t next_len,uint32_t start,uint32_t end,uint32_t distance);void sp_convolve_one(sp_sample_t* result,sp_sample_t* a,size_t a_len,sp_sample_t* b,size_t b_len);void sp_convolve(sp_sample_t* result,sp_sample_t* a,size_t a_len,sp_sample_t* b,size_t b_len,sp_sample_t* carryover,size_t carryover_len);