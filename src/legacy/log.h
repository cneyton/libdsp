#ifndef LOG_H_
#define LOG_H_

#include <stdbool.h>
#include <unistd.h>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

// -------------------------------- Debug logs --------------------------------
//
// To use debug log, you must define DEBUG_LOG before including log.h
//
// debug_add(const char * format, ...)


// -------------------------------- Log macros ---------------------------------
//
// log_add(const char * format, ...)
// log_zero(value_to_check,const char * format, ...)
// log_null(value_to_check, const char * format, ...)


// ----------------------------- common_die macros -----------------------------
//
// common_die         (return_value, const char * format, ...)
// common_die_void    (const char * format, ...)
// common_die_goto    (value_to_set, return_value, goto_flag, const char * format, ...)
//
// common_die_zero         (value_to_check, return_value, const char * format, ...)
// common_die_zero_void    (value_to_check, const char * format, ...)
// common_die_zero_goto    (value_to_check, value_to_set, return_value, goto_flag, const char * format, ...)
//
// common_die_null         (value_to_check, return_code, const char * format, ...)
// common_die_null_void    (value_to_check, const char * format, ...)
// common_die_null_goto    (value_to_check, value_to_set, return_value, goto_flag, const char * format, ...)
//
// common_die_magic         (value_to_check, expected_magic, return_code, const char * format, ...)
// common_die_magic_void    (value_to_check, expected_magic,  const char * format, ...)
// common_die_magic_goto    (value_to_check, expected_magic,  value_to_set, return_value, goto_flag, const char * format, ...)
//


void _log_add(const char *c_filename,
              const char *function_name,
              int line_number,
              const char *format, ...);


/******************************************************************************/
/*                                  debug_add                                 */
/******************************************************************************/
#ifdef DEBUG_LOG
    #warning #########################################################################
    #warning ######################### DEBUG LOG ENABLED !!! #########################
    #warning #########################################################################
    #define debug_add(...) fprintf(stderr, __VA_ARGS__)
#else
    #define debug_add(...)
#endif


/******************************************************************************/
/*                                   log_add                                  */
/******************************************************************************/
#define log_add(...)                                             \
    do {                                                         \
        _log_add(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
    } while(0)


/******************************************************************************/
/*                                  log_zero                                  */
/******************************************************************************/
#define log_zero(value_to_check, ...)      \
    if(unlikely(value_to_check < 0)) {     \
        log_add(__VA_ARGS__);              \
    }


/******************************************************************************/
/*                                  log_null                                  */
/******************************************************************************/
#define log_null(value_to_check, ...)      \
    if(unlikely(value_to_check == NULL)) { \
        log_add(__VA_ARGS__);              \
    }


/******************************************************************************/
/*                                  log_true                                  */
/******************************************************************************/
#define log_true(value_to_check, ...)      \
    if(unlikely(value_to_check == true)) { \
        log_add(__VA_ARGS__);              \
    }


/******************************************************************************/
/*                                 common_die                                 */
/******************************************************************************/
#define common_die(return_code, ...) {     \
    log_add(__VA_ARGS__);                  \
    return return_code;                    \
}


/******************************************************************************/
/*                               common_die_void                              */
/******************************************************************************/
#define common_die_void(...) {             \
    log_add(__VA_ARGS__);                  \
    return;                                \
}


/******************************************************************************/
/*                               common_die_goto                              */
/******************************************************************************/
#define common_die_goto(value_to_set, return_value, goto_flag, ...) \
    do{                                                             \
        log_add(__VA_ARGS__);                                       \
        value_to_set = return_value;                                \
        goto goto_flag;                                             \
    } while(0)


/******************************************************************************/
/*                              common_die_zero                               */
/******************************************************************************/
#define common_die_zero(value_to_check, return_code, ...) \
    do{                                                   \
        if(unlikely((value_to_check) < 0)) {              \
            log_add(__VA_ARGS__);                         \
            return return_code;                           \
        }                                                 \
    } while(0)


/******************************************************************************/
/*                            common_die_zero_void                            */
/******************************************************************************/
#define common_die_zero_void(value_to_check, ...) \
    do{                                           \
        if(unlikely(value_to_check < 0)) {        \
            log_add(__VA_ARGS__);                 \
            return;                               \
        }                                         \
    } while(0)


/******************************************************************************/
/*                           common_die_zero_goto                             */
/******************************************************************************/
#define common_die_zero_goto(value_to_check, value_to_set, return_value, goto_flag, ...) \
    do{                                                                                  \
        if(unlikely(value_to_check < 0)) {                                               \
            log_add(__VA_ARGS__);                                                        \
            value_to_set = return_value;                                                 \
            goto goto_flag;                                                              \
        }                                                                                \
    } while(0)


/******************************************************************************/
/*                              common_die_null                               */
/******************************************************************************/
#define common_die_null(value_to_check, return_code, ...) \
    do{                                                   \
        if(unlikely(value_to_check == NULL)) {            \
            log_add(__VA_ARGS__);                         \
            return return_code;                           \
        }                                                 \
    } while(0)


/******************************************************************************/
/*                            common_die_null_void                            */
/******************************************************************************/
#define common_die_null_void(value_to_check, ...) \
    do{                                           \
        if(unlikely(value_to_check == NULL)) {    \
            log_add(__VA_ARGS__);                 \
            return;                               \
        }                                         \
    } while(0)


/******************************************************************************/
/*                           common_die_null_goto                             */
/******************************************************************************/
#define common_die_null_goto(value_to_check, value_to_set, return_value, goto_flag, ...) \
    do{                                                                                  \
        if(unlikely(value_to_check == NULL)) {                                           \
            log_add(__VA_ARGS__);                                                        \
            value_to_set = return_value;                                                 \
            goto goto_flag;                                                              \
        }                                                                                \
    } while(0)



/******************************************************************************/
/*                              common_die_magic                              */
/******************************************************************************/
#define common_die_magic(value_to_check, expected_magic, return_code, ...) \
    do{                                                                    \
        if(unlikely(value_to_check == NULL)) {                             \
            log_add(__VA_ARGS__);                                          \
            return return_code;                                            \
        }                                                                  \
        if(unlikely(*((uint32_t *)value_to_check) != expected_magic)) {    \
            log_add(__VA_ARGS__);                                          \
            return return_code;                                            \
        }                                                                  \
    } while(0)


/******************************************************************************/
/*                            common_die_magic_void                           */
/******************************************************************************/
#define common_die_magic_void(value_to_check, expected_magic, ...)      \
    do{                                                                 \
        if(unlikely(value_to_check == NULL)) {                          \
            log_add(__VA_ARGS__);                                       \
            return ;                                                    \
        }                                                               \
        if(unlikely(*((uint32_t *)value_to_check) != expected_magic)) { \
            log_add(__VA_ARGS__);                                       \
            return ;                                                    \
        }                                                               \
    } while(0)


/******************************************************************************/
/*                           common_die_magic_goto                            */
/******************************************************************************/
#define common_die_magic_goto(value_to_check, expected_magic, value_to_set, return_value, goto_flag, ...) \
    do{                                                                                                   \
        if(unlikely(value_to_check == NULL)) {                                                            \
            log_add(__VA_ARGS__);                                                                         \
            value_to_set = return_value;                                                                  \
            goto goto_flag;                                                                               \
        }                                                                                                 \
        if(unlikely(*((uint32_t *)value_to_check) != expected_magic)) {                                   \
            log_add(__VA_ARGS__);                                                                         \
            value_to_set = return_value;                                                                  \
            goto goto_flag;                                                                               \
        }                                                                                                 \
    } while(0)


/******************************************************************************/
/*                            common_die_snprintf                             */
/******************************************************************************/
#define common_die_snprintf(value_to_check, return_code, max_size, ...) \
    do{                                                                 \
        if(unlikely(value_to_check < 0)) {                              \
            log_add(__VA_ARGS__);                                       \
            return return_code;                                         \
        }                                                               \
        if (unlikely(max_size <= (uint32_t) value_to_check)) {          \
            log_add(__VA_ARGS__);                                       \
        }                                                               \
    } while(0)


/******************************************************************************/
/*                          common_die_snprintf_void                          */
/******************************************************************************/
#define common_die_snprintf_void(value_to_check, max_size, ...) \
    do{                                                         \
        if(unlikely(value_to_check < 0)) {                      \
            log_add(__VA_ARGS__);                               \
            return;                                             \
        }                                                       \
        if (unlikely(max_size <= (uint32_t) value_to_check)) {  \
            log_add(__VA_ARGS__);                               \
        }                                                       \
    } while(0)

/******************************************************************************/
/*                         common_die_snprintf_goto                           */
/******************************************************************************/
#define common_die_snprintf_goto(value_to_check, value_to_set, return_value, max_size, goto_flag, ...) \
    do{                                                                                  \
        if(unlikely(value_to_check < 0)) {                                               \
            log_add(__VA_ARGS__);                                                        \
            value_to_set = return_value;                                                 \
            goto goto_flag;                                                              \
        }                                                                                \
        if (unlikely(max_size <= (uint32_t) value_to_check)) {                           \
            log_add(__VA_ARGS__);                                                        \
        }                                                                                \
    } while(0)


#endif

