#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define STATEMACHINE_NAME_MAX_LENGTH        64
#define STATEMACHINE_MAX_STATE_TRANSITIONS  20
#define STATEMACHINE_MAX_STATE              30

#define STATE_MACHINE_GOTO_NEXT_STATE       1
#define STATE_MACHINE_STAY_CUR_STATE        0

#define STATE_BEGIN(state_name)             \
    {                                       \
        .ID    = state_name,                \
        .name  = #state_name,               \
        .transitions_list = {
#define STATE_END     {NULL, 0}             \
                            }               \
    },

typedef struct statemachine T_statemachine;
typedef int (* T_transition_handler)(T_statemachine * sm);

typedef struct {
    T_transition_handler handler;
    uint16_t             new_state;
} T_transition;


typedef struct {
    uint16_t     ID;
    char         name[STATEMACHINE_NAME_MAX_LENGTH];
    T_transition transitions_list[STATEMACHINE_MAX_STATE_TRANSITIONS];
} T_state;

typedef struct statemachine {
    char             name[STATEMACHINE_NAME_MAX_LENGTH];
    uint16_t         number_of_states;
    uint32_t         nb_loop_in_curent_state;
    uint32_t         initial_state;
    T_state        * state_current;
    T_state        * states_list;
    bool             loop_detection;
    bool             debugging;
    bool             enable;
} T_statemachine;


int statemachine_init(T_statemachine *sm, T_state *list, uint16_t nb_states,
                      const char *name, uint32_t initial_state, bool enable);
int statemachine_reinit(T_statemachine *sm);
int statemachine_wakeup(T_statemachine * sm);

int statemachine_get_curstate(T_statemachine * sm);
int statemachine_get_nbloop(T_statemachine *sm);
int statemachine_disable_loop_detection(T_statemachine *sm);
int statemachine_display_trace(T_statemachine *sm);
int statemachine_enable(T_statemachine *sm);
int statemachine_disable(T_statemachine *sm);

#endif /* STATEMACHINE_H */
