
#define debug_log_p 1u
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
#if debug_log_p

#define debug_log(format,...) fprintf(stderr,"%s:%d " format "\n",__func__,__LINE__,__VA_ARGS__)

#else

#define debug_log(format,...) null

#endif
#define null (b0)(0u)
#define _readonly const
#define _noalias restrict
#define increment_one(a) a=(1u+a)
#define decrement_one(a) a=(a-1u)
#define local_memory_init(size) b0* _local_memory_addresses[size];b8 _local_memory_index=0u
#define local_memory_add(pointer) *(_local_memory_addresses+_local_memory_index)=pointer;_local_memory_index=(1u+_local_memory_index)
#define local_memory_free() while(_local_memory_index){decrement_one(_local_memory_index);free((*(_local_memory_addresses+_local_memory_index)));}
#define local_error_init b32_s local_error_number;b8 local_error_module
#define local_error(module_identifier,error_identifier) local_error_module=module_identifier;local_error_number=error_identifier;goto error
#define local_error_assert_enable 1u
#define sph 1u
#if local_error_assert_enable

#define local_error_assert(module,number,expr) if(!expr){local_error(module,number);}

#else

#define local_error_assert(module,number,expr) null

#endif
#define error_memory -1
#define error_input -2
char* error_description(b32_s n){return(((error_memory==n)?"memory":((error_input==n)?"input":"unknown")));}
#define local_define_malloc(variable_name,type) type* variable_name=malloc(sizeof(type));if(!variable_name){local_error(sph,error_memory);}
;
#define scm_tail SCM_CDR
#define scm_first SCM_CAR
#define scm_c_define_procedure_c(scm_temp,name,required,optional,rest,c_function,documentation) scm_temp=scm_c_define_gsubr(name,required,optional,rest,c_function);scm_set_procedure_property_x(scm_temp,scm_from_locale_symbol("documentation"),scm_from_locale_string(documentation))
#define scm_c_list_each(list,e,body) while(!scm_is_null(list)){e=scm_first(list);body;list=scm_tail(list);}
#define false_if_undefined(a) ((SCM_UNDEFINED==a)?SCM_BOOL_F:a)
#define null_if_undefined(a) ((SCM_UNDEFINED==a)?0u:a)
#define scm_is_list_false_or_undefined(a) (scm_is_true(scm_list_p(a))||(SCM_BOOL_F==a)||(SCM_UNDEFINED==a))
#define scm_is_integer_false_or_undefined(a) (scm_is_integer(a)||(SCM_BOOL_F==a)||(SCM_UNDEFINED==a))
SCM scm_c_bytevector_take(size_t size_octets,b8* a){SCM r=scm_c_make_bytevector(size_octets);memcpy(SCM_BYTEVECTOR_CONTENTS(r),a,size_octets);return(r);}
#define scm_if_undefined_expr(a,b,c) ((SCM_UNDEFINED==a)?b:c)
#define scm_if_undefined(a,b,c) if((SCM_UNDEFINED==a)){b;}else{c;}
#define scm_c_local_error_init SCM local_error_name;SCM local_error_data
#define scm_c_local_error(i,d) local_error_name=scm_from_locale_symbol(i);local_error_data=d;goto error
#define scm_c_local_error_create() scm_call_2(scm_make_error,local_error_name,(local_error_data?local_error_data:SCM_BOOL_F))
#define scm_c_local_define_malloc(variable_name,type) type* variable_name=malloc(sizeof(type));if(!variable_name){scm_c_local_error("memory",0u);}
#define scm_c_local_error_return() return(scm_c_local_error_create())
#if local_error_assert_enable

#define scm_c_local_error_assert(name,expr) if(!expr){scm_c_local_error(name,0u);}

#else

#define scm_c_local_error_assert(name,expr) null

#endif
SCM scm_make_error;SCM scm_error_p;SCM scm_error_name;SCM scm_error_data;b0 init_scm(){SCM m=scm_c_resolve_module("sph");scm_make_error=scm_variable_ref(scm_c_module_lookup(m,"make-error-p"));scm_error_name=scm_variable_ref(scm_c_module_lookup(m,"error-name-p"));scm_error_data=scm_variable_ref(scm_c_module_lookup(m,"error-data-p"));scm_error_p=scm_variable_ref(scm_c_module_lookup(m,"error?-p"));}
#define scm_c_local_error_glibc(error_number) scm_c_local_error("glibc",scm_from_locale_string(strerror(error_number)))
#define scm_c_require_success_glibc(a) s=a;if((s<0u)){scm_c_local_error_glibc(s);}
;
#include <string.h>
#define memory_copy memcpy
#define string_compare strcmp
#define string_break strpbrk
#define string_span strcspn
#define string_index_ci strcasestr
#define string_duplicate_n strndup
#define string_duplicate strdup
#define string_index_string strstr
#define string_index_right strrchr
#define string_index strchr
#define string_concat_n strncat
#define string_concat strcat
#define string_copy_n strncpy
#define string_copy strcpy
#define string_length_n strnlen
#define string_length strlen
#define file_exists_p(path) !(access(path,F_OK)==-1)
char* ensure_trailing_slash(char* str){b8 str_len=string_length(str);if((!str_len||('/'==(*(str+(str_len-1u)))))){return(str);}else{char* new_str=malloc((2u+str_len));memory_copy(new_str,str,str_len);memory_copy((new_str+str_len),"/",1u);*(new_str+(1u+str_len))=0u;return(new_str);}}
#define array_contains_s(array_start,array_end,search_value,index_temp,res) index_temp=array_start;res=0u;while((index_temp<=array_end)){if(((*index_temp)==search_value)){res=1u;break;}increment_one(index_temp);}
#define require_goto(a,label) if(!a){goto label;}
#if stability_typechecks

#define if_typecheck(expr,action) if(!expr){debug_log("type check failed %s",((string_length(#expr)<24u)?#expr:""));action;}

#else

#define if_typecheck(expr,action) null

#endif
;
#define init_status b8_s s
#define optional_samples_per_second(a) ((SCM_UNDEFINED==a)?96000u:scm_to_uint32(a))
scm_t_bits scm_type_sp_port;
#include <fcntl.h>
#define sp_port_type_alsa 0u
#define sp_port_type_file 1u
typedef struct{b32 samples_per_second;b32 channel_count;b32 open_flags;b8 type;pointer data;} port_data_t;SCM scm_c_create_sp_port(b8 type,b32_s open_flags,b32 samples_per_second,b32 channel_count,pointer data){port_data_t* port_data=scm_gc_malloc(sizeof(port_data_t),"sp-port-data");(*port_data).open_flags=open_flags;(*port_data).type=type;(*port_data).samples_per_second=samples_per_second;(*port_data).channel_count=channel_count;return(scm_new_smob(scm_type_sp_port,data));}
#define scm_c_require_success_alsa(a) s=a;if(s){scm_c_local_error("alsa",scm_from_locale_string(snd_strerror(s)));}
SCM scm_sp_io_alsa_open(SCM input_port_p,SCM channel_count,SCM device_name,SCM samples_per_second,SCM latency){scm_c_local_error_init;local_memory_init(1u);init_status;snd_pcm_t* alsa_port;char* device_name_c;scm_if_undefined(device_name,device_name_c="default",device_name_c=scm_to_locale_string(device_name);local_memory_add(device_name_c););scm_c_require_success_alsa(snd_pcm_open(&alsa_port,device_name_c,SND_PCM_STREAM_PLAYBACK,0u));b32 latency_c=scm_if_undefined_expr(latency,50u,scm_to_uint32(latency));b32 channel_count_c=scm_if_undefined_expr(channel_count,2u,scm_to_uint32(channel_count));b32 samples_per_second_c=scm_if_undefined_expr(samples_per_second,48000u,scm_to_uint32(samples_per_second));scm_c_require_success_alsa(snd_pcm_set_params(alsa_port,SND_PCM_FORMAT_U32,SND_PCM_ACCESS_RW_INTERLEAVED,channel_count_c,samples_per_second_c,0u,latency_c));return(scm_c_create_sp_port(sp_port_type_alsa,(input_port_p?O_RDONLY:O_WRONLY),samples_per_second_c,channel_count_c,(pointer)(alsa_port)));error:if(alsa_port){snd_pcm_close(alsa_port);}local_memory_free();scm_c_local_error_return();}b8_s file_au_read_header(b32_s file,b32* encoding,b32* samples_per_second,b32* channel_count){ssize_t s;b32 header[6];s=read(file,&header,24u);if(!((s==24u)&&((*header)==__builtin_bswap32(779316836u)))){return(-1);}lseek(file,__builtin_bswap32(*(header+1u)),SEEK_SET);(*encoding)=__builtin_bswap32(*(header+3u));(*samples_per_second)=__builtin_bswap32(*(header+4u));(*channel_count)=__builtin_bswap32(*(header+5u));return(0u);}b8_s file_au_write_header(b32_s file,b32 encoding,b32 samples_per_second,b32 channel_count){ssize_t s;b32 header[7];(*header)=__builtin_bswap32(779316836u);*(header+1u)=__builtin_bswap32(28u);*(header+2u)=__builtin_bswap32(4294967295u);*(header+3u)=__builtin_bswap32(encoding);*(header+4u)=__builtin_bswap32(samples_per_second);*(header+5u)=__builtin_bswap32(channel_count);*(header+6u)=0u;s=write(file,&header,28u);if(!(s==24u)){return(-1);}return(0u);}SCM scm_sp_io_file_open(SCM path,SCM open_flags,SCM channel_count,SCM samples_per_second){scm_c_local_error_init;scm_c_local_error_assert("input",(scm_is_string(path)&&scm_is_integer(open_flags)&&scm_is_integer(channel_count)&&scm_is_integer(samples_per_second)));local_memory_init(1u);init_status;b32_s file;SCM r;b32_s open_flags_c=scm_to_int32(open_flags);b32 samples_per_second_c;b32 channel_count_c;char* path_c=scm_to_locale_string(path);local_memory_add(path_c);if(file_exists_p(path_c)){file=open(path_c,open_flags_c);scm_c_require_success_glibc(file);b32 encoding;s=file_au_read_header(file,&encoding,&samples_per_second_c,&channel_count_c);if((s||!(encoding==6u))){scm_c_local_error("encoding",scm_from_uint32(encoding));}r=scm_c_create_sp_port(sp_port_type_file,open_flags_c,samples_per_second_c,channel_count_c,file);}else{file=open(path_c,(O_CREAT|open_flags_c),384u);scm_c_require_success_glibc(file);samples_per_second_c=optional_samples_per_second(samples_per_second);channel_count_c=scm_to_uint32(channel_count);s=file_au_write_header(file,6u,samples_per_second_c,channel_count_c);if((s<0u)){scm_c_local_error("write-header",0u);}r=scm_c_create_sp_port(sp_port_type_file,open_flags_c,samples_per_second_c,channel_count_c,file);}local_memory_free();return(r);error:local_memory_free();scm_c_local_error_return();}SCM scm_sp_io_ports_close(SCM a){scm_c_local_error_init;SCM e;port_data_t* port_data;init_status;scm_c_list_each(a,e,{if(SCM_SMOB_PREDICATE(scm_type_sp_port,e)){port_data=(port_data_t*)(SCM_SMOB_DATA(e));if((sp_port_type_alsa==(*port_data).type)){scm_c_require_success_alsa(snd_pcm_close((snd_pcm_t*)((*port_data).data)));}else{if((sp_port_type_file==(*port_data).type)){scm_c_require_success_glibc(close((b32)((*port_data).data)));}}}});error:scm_c_local_error_return();}size_t scm_c_type_sp_port_free(SCM a){scm_sp_io_ports_close(scm_list_1(a));return(0u);}SCM scm_sp_io_stream(SCM input_ports,SCM output_ports,SCM segment_size,SCM prepared_segment_count,SCM proc,SCM user_state){b32 i=scm_to_uint32(prepared_segment_count);SCM prepared_segments=SCM_EOL;SCM time_scm=scm_from_uint8(0u);SCM segment_size_octets=scm_product(segment_size,scm_from_uint8(8u));SCM zero=scm_from_uint8(0u);while(i){scm_cons(scm_make_f32vector(segment_size_octets,zero),prepared_segments);decrement_one(i);}SCM output;loop:output=scm_apply_2(proc,time_scm,prepared_segments,user_state);if(!scm_is_true(output)){return(SCM_UNSPECIFIED);}time_scm=scm_sum(segment_size,time_scm);goto loop;;return(SCM_UNSPECIFIED);}b0 init_sp(){init_scm();scm_type_sp_port=scm_make_smob_type("sp-port",0u);scm_set_smob_free(scm_type_sp_port,scm_c_type_sp_port_free);SCM t;scm_c_define_procedure_c(t,"sp-io-stream",0u,1u,1u,scm_sp_io_stream,"list list integer integer procedure ->\n    (sp-port ...) (sp-port ...) samples-per-segment prepared-segment-count {integer:time list:prepared-segments user-state ...} ->");scm_c_define_procedure_c(t,"sp-io-ports-close",1u,0u,0u,scm_sp_io_ports_close,"(sp-port ...) ->");scm_c_define_procedure_c(t,"sp-io-file-open",2u,2u,0u,scm_sp_io_file_open,"path mode [channel-count samples-per-second] -> sp-port\n    string integer integer integer -> sp-port");scm_c_define_procedure_c(t,"sp-io-alsa-open",2u,3u,0u,scm_sp_io_alsa_open,"input-port? channel-count device-name samples-per-second latency -> sp-port\n    boolean integer string integer integer");}