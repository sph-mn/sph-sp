#include <stdio.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "./main/sph-sp.h"
#include "./foreign/sph/one.c"
#include "./foreign/sph/local-memory.c"
#include <foreign/kiss_fft.h>
#include <foreign/tools/kiss_fftr.h>
#define sp_status_declare status_declare_group(sp_status_group_sp)
#define sp_system_status_require_id(id) if(id<0){status_set_both_goto(sp_status_group_libc,id);}
#define sp_system_status_require(expression) status_set_id(expression);if(status.id<0){status_set_group_goto(sp_status_group_libc);}else{status_reset;}
#define sp_alsa_status_require(expression) status_set_id(expression);if(status_is_failure){status_set_group_goto(sp_status_group_alsa);}
#define sp_status_require_alloc(a) if(!a){status_set_both_goto(sp_status_group_sp,sp_status_id_memory);}
#define inc(a) a=(1+a)
#define dec(a) a=(a-1)
enum{sp_status_group_alsa,sp_status_group_libc,sp_status_group_sndfile,sp_status_group_sp,sp_status_id_file_channel_mismatch,sp_status_id_file_encoding,sp_status_id_file_header,sp_status_id_file_incompatible,sp_status_id_file_incomplete,sp_status_id_eof,sp_status_id_input_type,sp_status_id_memory,sp_status_id_not_implemented,sp_status_id_port_closed,sp_status_id_port_position,sp_status_id_port_type,sp_status_id_undefined};uint8_t* sp_status_description(status_t a){return(((sp_status_group_sp==a.group)?((uint8_t*)(case*(=,(a.id),(sp_status_id_eof("end of file")),(sp_status_id_input_type("input argument is of wrong type")),(sp_status_id_not_implemented("not implemented")),(sp_status_id_memory("memory allocation error")),(sp_status_id_file_incompatible("file channel count or sample rate is different from what was requested")),(sp_status_id_file_incomplete("incomplete write")),(sp_status_id_port_type("incompatible port type")),(else(""))))):((sp_status_group_alsa==a.group)?((uint8_t*)(sf_error_number((a.id)))):((sp_status_group_sndfile==a.group)?((uint8_t*)(sf_error_number((a.id)))):((uint8_t*)(""))))));};uint8_t* sp_status_name(status_t a){return(((sp_status_group_sp==a.group)?((uint8_t*)(case*(=,(a.id),(sp_status_id_input_type("input-type")),(sp_status_id_not_implemented("not-implemented")),(sp_status_id_memory("memory")),(else("unknown"))))):((sp_status_group_alsa==a.group)?((uint8_t*)("alsa")):((sp_status_group_sndfile==a.group)?((uint8_t*)("sndfile")):((uint8_t*)("unknown"))))));};
/** return an array for channels with data arrays for each channel.
  returns zero if memory could not be allocated */
sp_sample_t** sp_alloc_channel_array(uint32_t channel_count,uint32_t sample_count){local_memory_init((channel_count+1));sp_sample_t* channel;sp_sample_t** result;result=malloc((channel_count*sizeof(sp_sample_t*)));if(!result){return(0);};local_memory_add(result);while(channel_count){dec(channel_count);channel=calloc((sample_count*sizeof(sp_sample_t)),1);if(!channel){local_memory_free;return(0);};local_memory_add(channel);result[channel_count]=channel;};return(result);};
/** lower precision version of sin() that is faster to compute */
sp_sample_t sp_sin_lq(sp_sample_t a){sp_sample_t b;sp_sample_t c;b=(4/M_PI);c=(-4/(M_PI*M_PI));return((((b*a)+(c*a*abs(a)))));};
/** the normalised sinc function */
sp_sample_t sp_sinc(sp_sample_t a){return(((0==a)?1:(sin((M_PI*a))/(M_PI*a))));};
/* write samples for a sine wave into dest.
   sample-duration: in seconds
   freq: radian frequency
   phase: phase offset
   amp: amplitude. 0..1
   defines sp-sine, sp-sine-lq */
#define define_sp_sine(id,sin) void id(sp_sample_t* dest,uint32_t len,sp_float_t sample_duration,sp_float_t freq,sp_float_t phase,sp_float_t amp){uint32_t index=0;while((index<=len)){dest[index]=(amp*sin((freq*phase*sample_duration)));inc(phase);inc(index);};}
define_sp_sine(sp_sine,sin);define_sp_sine(sp_sine_lq,sp_sin_lq);status_t sp_fft(sp_sample_t* result,uint32_t result_len,sp_sample_t* source,uint32_t source_len){sp_status_declare;kiss_fftr_cfg fftr_state;local_memory_init(2);fftr_state=kiss_fftr_alloc(result_len,0,0,0);if(!fftr_state){status_set_id_goto(sp_status_id_memory);};local_memory_add(fftr_state);sp_alloc_define(out,kiss_fft_cpx*,(result_len*sizeof(kiss_fft_cpx)));local_memory_add(out);kiss_fftr(fftr_state,source,out);while(result_len){dec(result_len);result[result_len]=(out[result_len]).r;};exit:local_memory_free;return(status);};status_t sp_ifft(sp_sample_t* result,uint32_t result_len,sp_sample_t* source,uint32_t source_len){sp_status_declare;kiss_fftr_cfg fftr_state;local_memory_init(2);fftr_state=kiss_fftr_alloc(source_len,1,0,0);if(!fftr_state){status_set_id_goto(sp_status_id_memory);};local_memory_add(fftr_state);sp_alloc_define(in,kiss_fft_cpx*,(source_len*sizeof(kiss_fft_cpx)));local_memory_add(in);while(source_len){dec(source_len);(in[source_len]).r=source[(source_len*sizeof(sp_sample_t))];};kiss_fftri(fftr_state,in,result);exit:local_memory_free;return(status);};
/** apply a centered moving average filter to source at index start to end inclusively and write to result.
  removes higher frequencies with little distortion in the time domain.
   * only the result portion corresponding to the subvector from start to end is written to result
   * prev and next are unprocessed segments and can be null pointers,
     for example at the beginning and end of a stream
   * since the result value for a sample is calculated using samples left and right of it,
     a previous and following part of a stream is eventually needed to reference values
     outside the source segment to create a valid continuous result.
     zero is used for unavailable values outside the source segment
   * available values outside the start/end range are considered where needed to calculate averages
   * rounding errors are kept low by using modified kahan neumaier summation and not using a
     recursive implementation. both properties which make it much slower than many other implementations */
status_i_t sp_moving_average(sp_sample_t* result,sp_sample_t* source,uint32_t source_len,sp_sample_t* prev,uint32_t prev_len,sp_sample_t* next,uint32_t next_len,uint32_t start,uint32_t end,uint32_t radius){uint32_t left;uint32_t right;uint32_t width;sp_sample_t* window;uint32_t window_index;if(!source_len){return(1);};width=(1+(2*radius));window=0;if(!((start>=radius)&&((start+radius+1)<=source_len))){window=malloc((width*sizeof(sp_sample_t)));if(!window){return(1);};};while((start<=end)){if((start>=radius)&&((start+radius+1)<=source_len)){*result=(sp_sample_sum(((source+start)-radius),width)/width);}else{window_index=0;if(start<radius){right=(radius-start);if(prev){left=((right>prev_len)?0:(prev_len-right));while((left<prev_len)){window[window_index]=prev[left];inc(window_index);inc(left);};};while((window_index<right)){window[window_index]=0;inc(window_index);};left=0;}else{left=(start-radius);};right=(start+radius);if(right>=source_len){right=(source_len-1);};while((left<=right)){window[window_index]=source[left];inc(window_index);inc(left);};right=(start+radius);if((right>=source_len)&&next){left=0;right=(right-source_len);if(right>=next_len){right=(next_len-1);};while((left<=right)){window[window_index]=next[left];inc(window_index);inc(left);};};while((window_index<width)){window[window_index]=0;inc(window_index);};*result=(sp_sample_sum(window,width)/width);};inc(result);inc(start);};free(window);return(0);};sp_float_t sp_window_blackman(sp_float_t a,size_t width){return(((0.42-(0.5*cos(((2*M_PI*a)/(width-1)))))+(0.8*cos(((4*M_PI*a)/(width-1))))));};
/** modify an impulse response kernel for spectral inversion.
   a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response top to bottom */
void sp_spectral_inversion_ir(sp_sample_t* a,size_t a_len){size_t center;while(a_len){dec(a_len);a[a_len]=(-1*a[a_len]);};center=((a_len-1)/2);inc((a[center]));};
/** inverts the sign for samples at odd indexes.
  a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response left to right */
void sp_spectral_reversal_ir(sp_sample_t* a,size_t a_len){while((a_len>1)){a_len=(a_len-2);a[a_len]=(-1*a[a_len]);};};
/** discrete linear convolution.
  result length must be at least a-len + b-len - 1 */
void sp_convolve_one(sp_sample_t* result,sp_sample_t* a,size_t a_len,sp_sample_t* b,size_t b_len){size_t a_index;size_t b_index;a_index=0;b_index=0;while((a_index<a_len)){while((b_index<b_len)){result[(a_index+b_index)]=(result[(a_index+b_index)]+(a[a_index]*b[b_index]));inc(b_index);};b_index=0;inc(a_index);};};
/** discrete linear convolution for segments of a continuous stream. maps segments (a, a-len) to result.
  result length is a-len, carryover length is b-len or previous b-len. b-len must be greater than zero */
void sp_convolve(sp_sample_t* result,sp_sample_t* a,size_t a_len,sp_sample_t* b,size_t b_len,sp_sample_t* carryover,size_t carryover_len){size_t size;size_t a_index;size_t b_index;size_t c_index;memset(result,0,(a_len*sizeof(sp_sample_t)));memcpy(result,carryover,(carryover_len*sizeof(sp_sample_t)));memset(carryover,0,(b_len*sizeof(sp_sample_t)));size=((a_len<b_len)?0:(a_len-(b_len-1)));if(size){sp_convolve_one(result,a,size,b,b_len);};a_index=size;b_index=0;while((a_index<a_len)){while((b_index<b_len)){c_index=(a_index+b_index);if(c_index<a_len){result[c_index]=(result[c_index]+(a[a_index]*b[b_index]));}else{c_index=(c_index-a_len);carryover[c_index]=(carryover[c_index]+(a[a_index]*b[b_index]));};inc(b_index);};b_index=0;inc(a_index);};};
#include "./main/windowed-sinc.c"
#include "./main/io.sc"
