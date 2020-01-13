#ifndef LIST_MANAGER_H
#define LIST_MANAGER_H

#include <pthread.h>

#include "log.h"
#include "list.h"


/*
 * This macro generate all list functions :
 *     _get_first_XXXXXX  ( )
 *     _get_next_XXXXXX   ( )
 *     _get_prev_XXXXXX   ( )
 *     _is_last_XXXXXX    ( )
 *     _is_first_XXXXXX   ( )
 *     _reset_list_XXXXXX ( )
 *     _get_nb_XXXXXX     ( )
 *   with XXXXXX the patern pass in argument.
 *
 * Arguments :
 *     fct_patern : Function patern (XXXXXX)
 *     info       : [T_list_manager_info] Information necessary for list_manager
 *     type       : The type of the struct this is embedded in list.
 *     member     : The name of the list_struct within the struct.
 *
 * Example :
 *     You can find an example in PRO12/src/core/memory/interventions_engine.c
 * ##################################################
 * #define LIST_BUFFER_SIZE 10
 *
 * typedef struct {
 *     struct list_head node;
 *     int32_t data;
 * } T_list_buffer;
 *
 * struct {
 *     pthread_mutex_t mutex;
 *     T_list_manager_info list;
 *     T_list_buffer list_buffer[LIST_BUFFER_SIZE];
 * } g_module = {
 *     .mutex = PTHREAD_MUTEX_INITIALIZER,
 *     .list = {
 *         .buffer = g_module.list_buffer,
 *         .buffer_size = LIST_BUFFER_SIZE,
 *         .mutex = &g_module.mutex,
 *     },
 * };
 *
 * GENERATE_LIST_FUNCTIONS(module, g_module.list, T_list_buffer, node);
 * ##################################################
 *
 */


typedef struct {
    struct list_head head;    /*!< The head for your list */
    void *buffer;             /*!< Buffer used for your list */
    int buffer_size;          /*!< Buffer size */
    pthread_mutex_t *mutex;   /*!< Mutex protecting your list data */
} T_list_manager_info;


#define GENERATE_LIST_FUNCTIONS(fct_patern, info, type, member)                                         \
    static void * _get_first_##fct_patern( void ) {                                                     \
        int ret;                                                                                        \
        int count = 0;                                                                                  \
        type *elt;                                                                                      \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread( ret, NULL, "Error pthread_mutex_lock return %d", ret );                     \
        \
        list_count_entries( count, elt, &info.head, member );                                           \
        common_die_zero_unlock( count, NULL, info.mutex, "Error list_count_entries return %d", count ); \
        \
        list_get_first_entry( elt, &info.head, member );                                                \
        common_die_null_unlock( elt, NULL, info.mutex, "Error first element NULL" );                    \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread( ret, NULL, "Error pthread_mutex_unlock return %d", ret );                   \
        \
        return (void *) elt;                                                                            \
    }                                                                                                   \
    \
    static void * _get_next_##fct_patern( void *elt ) {                                                 \
        int ret;                                                                                        \
        type *current = elt;                                                                            \
        type *next;                                                                                     \
        \
        common_die_null( current, NULL, "null pointer" );                                               \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread( ret, NULL, "Error pthread_mutex_lock return %d", ret );                     \
        \
        next = list_get_next_entry( current, type, member );                                            \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread( ret, NULL, "Error pthread_mutex_unlock return %d", ret );                   \
        \
        return (void *) next;                                                                           \
    }                                                                                                   \
    \
    static void * _get_prev_##fct_patern( void *elt ) {                                                 \
        int ret;                                                                                        \
        type *current = elt;                                                                            \
        type *prev;                                                                                     \
        \
        common_die_null( current, NULL, "null pointer" );                                               \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread( ret, NULL, "Error pthread_mutex_lock return %d", ret );                     \
        \
        prev = list_get_prev_entry( current, type, member );                                            \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread( ret, NULL, "Error pthread_mutex_unlock return %d", ret );                   \
        \
        return (void *) prev;                                                                           \
    }                                                                                                   \
    \
    static bool _is_last_##fct_patern( void *elt ) {                                                    \
        int ret;                                                                                        \
        type *last;                                                                                     \
        \
        common_die_null( elt, false, "null pointer" );                                                  \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread( ret, false, "Error pthread_mutex_lock return %d", ret );                    \
        \
        list_get_last_entry( last, &info.head, member );                                                \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread( ret, false, "Error pthread_mutex_unlock return %d", ret );                  \
        \
        if( last == elt ) {                                                                             \
            return true;                                                                                \
        }                                                                                               \
        \
        return false;                                                                                   \
    }                                                                                                   \
    \
    static bool _is_first_##fct_patern( void *elt ) {                                                   \
        int ret;                                                                                        \
        type *first;                                                                                    \
        \
        common_die_null( elt, false, "null pointer" );                                                  \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread( ret, false, "Error pthread_mutex_lock return %d", ret );                    \
        \
        list_get_first_entry( first, &info.head, member );                                              \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread( ret, false, "Error pthread_mutex_unlock return %d", ret );                  \
        \
        if( first == elt ) {                                                                            \
            return true;                                                                                \
        }                                                                                               \
        \
        return false;                                                                                   \
    }                                                                                                   \
    \
    static void _reset_list_##fct_patern( void ) {                                                      \
        int ret;                                                                                        \
        int i;                                                                                          \
        type *elt = info.buffer;                                                                        \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread_void( ret, "Error pthread_mutex_lock return %d", ret );                      \
        \
        INIT_LIST_HEAD( &info.head );                                                                   \
        for( i = 0; i < info.buffer_size; i++ ) {                                                       \
            INIT_LIST_HEAD( &elt[i].member );                                                           \
        }                                                                                               \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread_void( ret, "Error pthread_mutex_unlock return %d", ret );                    \
    }                                                                                                   \
    \
    static int _get_nb_##fct_patern( void ) {                                                           \
        int ret, count = 0;                                                                             \
        type *elt;                                                                                      \
        \
        ret = pthread_mutex_lock( info.mutex );                                                         \
        common_die_pthread( ret, -1, "Error pthread_mutex_lock return %d", ret );                       \
        \
        list_count_entries( count, elt, &info.head, member );                                           \
        \
        ret = pthread_mutex_unlock( info.mutex );                                                       \
        common_die_pthread( ret, -2, "Error pthread_mutex_unlock return %d", ret );                     \
        \
        return count;                                                                                   \
    }                                                                                                   \


#endif

