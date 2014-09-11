
#include <stdio.h>
#include <libguile.h>
#include <alsa/asoundlib.h>
#include <inttypes.h>
#define b64_s int64_t
#define b32_s int32_t
#define b16_s int16_t
#define b8_s int8_t
#define b64 uint64_t
#define b32 uint32_t
#define b16 uint16_t
#define b8 uint8_t
#define b0 void
#define pointer uintptr_t
#if verbose

#define debug_log(format,...) fprintf(stderr,"%s:%d " format "\n",__func__,__LINE__,__VA_ARGS__)

#else

#define debug_log(format,...) null

#endif
#define null (b0)(0u)
#define _readonly const
#define _noalias restrict
#define increment_one(a) a=(1u+a)
#define decrement_one(a) a=(a-1u)
;
#define scm_c_define_c(scm_temp,name,required,optional,rest,c_function,documentation) scm_temp=scm_c_define_gsubr(name,required,optional,rest,c_function);scm_set_procedure_property_x(scm_temp,scm_from_locale_symbol("documentation"),scm_from_locale_string(documentation))
#define scm_tail SCM_CDR
#define scm_first SCM_CAR
;
#define init_status b8_s s
#define sw_resampling 0u
typedef struct{SCM samples_per_second;SCM time;SCM frames_per_buffer;SCM channel_count;b0* io;} sp_state_t;typedef struct{snd_pcm_t* out;snd_pcm_t* in;} sp_io_alsa_t;sp_state_t sp_state;SCM scm_sp_init_alsa(SCM samples_per_second,SCM device_name,SCM channel_count,SCM latency){init_status;sp_io_alsa_t* io=malloc(sizeof(sp_io_alsa_t));if((0u==io)){debug_log("not enough memory available");}char* device_name_c=((SCM_UNDEFINED==device_name)?"default":scm_to_locale_string(device_name));s=snd_pcm_open(&(*io).out,device_name_c,SND_PCM_STREAM_PLAYBACK,0u);if((s<0u)){goto error;}s=snd_pcm_set_params((*io).out,SND_PCM_FORMAT_U32,SND_PCM_ACCESS_RW_INTERLEAVED,((SCM_UNDEFINED==channel_count)?2u:scm_to_uint32(channel_count)),((SCM_UNDEFINED==samples_per_second)?48000u:scm_to_uint32(samples_per_second)),sw_resampling,((SCM_UNDEFINED==samples_per_second)?25u:scm_to_uint8(latency)));if((s<0u)){goto error;}if(!(SCM_UNDEFINED==device_name)){free(device_name_c);}sp_state.io=io;return(SCM_BOOL_T);error:debug_log("alsa init error: %s",snd_strerror(s));if(!(SCM_UNDEFINED==device_name)){free(device_name_c);}free(io);return(SCM_BOOL_F);}
#define increment_time(sp_state) sp_state.time=scm_sum(sp_state.time,sp_state.frames_per_buffer)
SCM scm_sp_loop_alsa(SCM proc){SCM buffer;b32 frames_per_buffer_c=scm_to_uint32(sp_state.frames_per_buffer);sp_io_alsa_t io=(*(sp_io_alsa_t*)(sp_state.io));snd_pcm_sframes_t frames_written;loop:buffer=scm_call_2(proc,scm_c_make_bytevector(frames_per_buffer_c),sp_state.time);if(scm_is_true(buffer)){frames_written=snd_pcm_writei(io.out,SCM_BYTEVECTOR_CONTENTS(buffer),frames_per_buffer_c);if((frames_written==-EAGAIN)){goto loop;}else{if((frames_written<0u)){if((snd_pcm_recover(io.out,frames_written,0u)<0u)){debug_log("write error: %s",snd_strerror(frames_written));return(SCM_BOOL_F);}else{increment_time(sp_state);goto loop;}}else{increment_time(sp_state);goto loop;}}}else{return(SCM_BOOL_T);}}SCM scm_sp_deinit_alsa(){init_status;s=snd_pcm_close((*(sp_io_alsa_t*)(sp_state.io)).out);if((s<0u)){debug_log("write error: %s",snd_strerror(s));return(SCM_BOOL_F);}else{return(SCM_BOOL_T);}}b0 init_sp(){SCM scm_temp;scm_c_define_c(scm_temp,"sp-loop-alsa",1u,0u,0u,scm_sp_loop_alsa,"");scm_c_define_c(scm_temp,"sp-deinit-alsa",0u,0u,0u,scm_sp_deinit_alsa,"");scm_c_define_c(scm_temp,"sp-init-alsa",0u,4u,0u,scm_sp_init_alsa,"");}