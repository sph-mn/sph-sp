/* return status as integer code with group identifier
for exception handling with a local variable and a goto label
status id 0 is success, everything else can be considered a special case or failure
status ids are signed integers for compatibility with error return codes from other existing libraries
group ids are strings used to categorise sets of errors codes from different libraries for example */
typedef struct {
  int id;
  uint8_t* group;
} s_t;
#define s_success 0
#define s_group_undefined ((uint8_t*)(""))
#define s_declare s_t s_current = { s_success, s_group_undefined }
#define s_is_success (s_success == s_current.id)
#define s_is_failure !s_is_success
#define s_return return (s_current)
#define s_set(group_id, status_id) \
  s_current.group = ((uint8_t*)(group_id)); \
  s_current.id = status_id
#define s_set_goto(group_id, status_id) \
  s_set(group_id, status_id); \
  goto exit
#define s(expression) \
  s_current = expression; \
  if (s_is_failure) { \
    goto exit; \
  }
#define si(expression) \
  s_current.id = expression; \
  if (s_is_failure) { \
    goto exit; \
  }
