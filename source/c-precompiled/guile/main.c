#include <libguile.h>
#include "./guile/main.c"
#include "./foreign/guile/sph/guile.c"
#define inc(a) a=(1+a)
#define dec(a) a=(a-1)
/** raise an exception with error information set as an alist with given values */
#define scm_c_error(name,description) scm_call_1(scm_rnrs_raise,(scm_list_3((scm_from_latin1_symbol(name)),(scm_cons((scm_from_latin1_symbol("description")),(scm_from_utf8_string(description)))),(scm_cons((scm_from_latin1_symbol("c-routine")),(scm_from_latin1_symbol(__FUNCTION__)))))))
#define status_to_scm_error(a) scm_c_error((sp_status_name(a)),(sp_status_description(a)))
#define status_to_scm(result) (status_is_success?result:status_to_scm_error(status))
/** call scm-c-error if status is not success or return result */
#define status_to_scm_return(result) return((status_to_scm(result)))
#define scm_sp_object_type_init "sp-object is a guile smob that stores a sub type id and pointer";scm_type_sp_object=scm_make_smob_type("sp-object",0);scm_set_smob_print(scm_type_sp_object,scm_sp_object_print);scm_set_smob_free(scm_type_sp_object,scm_sp_object_free);
/** integer -> string */
#define sp_port_type_to_name(a) ((sp_port_type_file==a)?"file":((sp_port_type_alsa==a)?"alsa":"unknown"))
#define sp_object_type_port 0
#define sp_object_type_windowed_sinc 1
#define scm_sp_object_type SCM_SMOB_FLAGS
#define scm_sp_object_data SCM_SMOB_DATA
#define optional_sample_rate(a) (scm_is_undefined(a)?-1:scm_to_int32(a))
#define optional_channel_count(a) (scm_is_undefined(a)?-1:scm_to_int32(a))
scm_t_bits scm_type_sp_object;SCM scm_rnrs_raise;
/** get a guile scheme object for channel data sample arrays. returns a list of f64vectors.
  eventually frees given data arrays */
SCM scm_take_channel_data(sp_sample_t** a,uint32_t channel_count,uint32_t sample_count){SCM result;result=SCM_EOL;while(channel_count){dec(channel_count);result=scm_cons((scm_take_f64vector((a[channel_count]),sample_count)),result);};free(a);return(result);};
/** only the result array is allocated, data is referenced to the scm vectors */
sp_sample_t** scm_to_channel_data(SCM a,uint32_t* channel_count,size_t* sample_count){*channel_count=scm_to_uint32((scm_length(a)));if(!*channel_count){return(0);};sp_sample_t** result;size_t index;result=malloc((*channel_count*sizeof(sp_sample_t*)));if(!result){return(0);};*sample_count=sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH((scm_first(a)))));index=0;while(!scm_is_null(a)){result[index]=((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS((scm_first(a)))));inc(index);a=scm_tail(a);};return(result);};
/** sp-object type for storing arbitrary pointers */
SCM scm_sp_object_create(void* pointer,uint8_t sp_object_type){SCM result;result=scm_new_smob(scm_type_sp_object,((scm_t_bits)(pointer)));SCM_SET_SMOB_FLAGS(result,((scm_t_bits)(sp_object_type)));return(result);};int scm_sp_object_print(SCM a,SCM output_port,scm_print_state* print_state){char* result;uint8_t type;sp_port_t* sp_port;result=malloc((70+10+7+10+10+2+2));if(!result){return(0);};type=scm_sp_object_type(a);if(sp_object_type_port==type){sp_port=((sp_port_t*)(scm_sp_object_data(a)));sprintf(result,"#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s input?:%s>",((void*)(a)),(sp_port_type_to_name((sp_port->type))),(sp_port->sample_rate),(sp_port->channel_count),((sp_port_bit_closed&sp_port->flags)?"#t":"#f"),((sp_port_bit_input&sp_port->flags)?"#t":"#f"));}else if(sp_object_type_windowed_sinc==type){sprintf(result,"#<sp-state %lx type:windowed-sinc>",((void*)(a)));}else{sprintf(result,"#<sp %lx>",((void*)(a)));};scm_display((scm_take_locale_string(result)),output_port);return(0);};size_t scm_sp_object_free(SCM a){uint8_t type;void* data;type=SCM_SMOB_FLAGS(a);data=((void*)(scm_sp_object_data(a)));if(sp_object_type_windowed_sinc==type){sp_windowed_sinc_state_destroy(data);}else if(sp_object_type_port==type){sp_port_close(data);};return(0);};SCM scm_float_nearly_equal_p(SCM a,SCM b,SCM margin){return((scm_from_bool((f64_nearly_equal_p((scm_to_double(a)),(scm_to_double(b)),(scm_to_double(margin)))))));};SCM scm_f64vector_sum(SCM a,SCM start,SCM end){return((scm_from_double((f64_sum(((scm_is_undefined(start)?0:scm_to_size_t(start))+((f64_s*)(SCM_BYTEVECTOR_CONTENTS(a)))),((scm_is_undefined(end)?SCM_BYTEVECTOR_LENGTH(a):(end-(1+start)))*sizeof(f64_s)))))));};SCM scm_f32vector_sum(SCM a,SCM start,SCM end){return((scm_from_double((f32_sum(((scm_is_undefined(start)?0:scm_to_size_t(start))+((f32_s*)(SCM_BYTEVECTOR_CONTENTS(a)))),((scm_is_undefined(end)?SCM_BYTEVECTOR_LENGTH(a):(end-(1+start)))*sizeof(f32_s)))))));};
#include "./guile/io.c"
#include "./guile/generate.c"
#include "./guile/transform.c"
void sp_init_guile(){init_sp_io();init_sp_generate();init_sp_transform();scm_c_define_procedure_c_init;scm_sp_object_type_init;scm_rnrs_raise=scm_c_public_ref("rnrs exceptions","raise");scm_c_define_procedure_c("f32vector-sum",1,2,0,scm_f32vector_sum,("f32vector [start end] -> number"));scm_c_define_procedure_c("f64vector-sum",1,2,0,scm_f64vector_sum,("f64vector [start end] -> number"));scm_c_define_procedure_c("float-nearly-equal?",3,0,0,scm_float_nearly_equal_p,("a b margin -> boolean\n    number number number -> boolean"));};