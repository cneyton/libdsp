#include <stdint.h>
#include <string.h>

#include "log.h"
#include "statemachine.h"

int statemachine_reinit(T_statemachine *sm)
{
    int ret, i;
    bool init_state_found = false;

    common_die_null(sm, -1, "sm null pointer");

    // search state corresponding to initial_state
    for (i = 0; i < sm->number_of_states; i++) {
        if (sm->states_list[i].ID == sm->initial_state) {
            sm->state_current  = &sm->states_list[i];
            init_state_found = true;
            break;
        }
    }
    if (init_state_found == false) {
        common_die(-2, "initial state [%d] has not been found -- stop statemachine %s",
                   sm->initial_state, sm->name);
    }

    sm->nb_loop_in_curent_state = 0;

    return 0;
}

int statemachine_init(T_statemachine *sm, T_state *list, uint16_t nb_states,
                      const char *name, uint32_t initial_state, bool enable)
{
    common_die_null(sm,   -1, "sm null pointer" );
    common_die_null(list, -2, "state list  null pointer" );
    common_die_null(name, -3, "name null pointer" );

    bool init_state_found = false;
    int ret, i;

    if (nb_states > STATEMACHINE_MAX_STATE)
        log_add("warrning : init of state machine od %d element seems too big",nb_states);

    strncpy(sm->name,name,STATEMACHINE_NAME_MAX_LENGTH);
    sm->name[STATEMACHINE_NAME_MAX_LENGTH-1]='\0';

    sm->number_of_states = nb_states;
    sm->debugging = false;
    sm->states_list = list;
    sm->initial_state = initial_state;

    // search state corresponding to initial_state
    for (i = 0; i < sm->number_of_states; i++) {
        if (sm->states_list[i].ID == initial_state) {
            sm->state_current  = &sm->states_list[i];
            init_state_found = true;

            if (sm->debugging == true)
                log_add("[%s] ==> %s\n", sm->name, sm->state_current->name);

            break;
        }
    }

    if (init_state_found == false)
        common_die(-8, "initial state [%d] has not been found -- stop statemachine %s",
                   initial_state, name);

    sm->nb_loop_in_curent_state = 0;
    sm->loop_detection = true;
    sm->enable = enable;

    return 0;
}

int statemachine_get_curstate(T_statemachine *sm)
{
    common_die_null(sm, -1, "sm null pointer");
    return sm->state_current->ID;
}

int statemachine_get_nbloop(T_statemachine *sm)
{
    common_die_null(sm, -1, "sm null pointer");
    return (int) sm->nb_loop_in_curent_state;
}

int statemachine_disable_loop_detection(T_statemachine *sm)
{
    common_die_null(sm, -1, "sm null pointer");
    sm->loop_detection = false;
    return 0;
}

int statemachine_display_trace(T_statemachine *sm)
{
    common_die_null(sm, -1, "sm null pointer");
    sm->debugging = true;
    return 0;
}

int statemachine_enable(T_statemachine *sm)
{
    common_die_null(sm, -1,"sm null pointer");
    sm->enable = true;
    return 0;
}

int statemachine_disable(T_statemachine *sm)
{
    common_die_null(sm, -1, "sm null pointer");
    sm->enable = false;
    return 0;
}

int statemachine_engine(T_statemachine *sm)
{
    int i, ret;
    uint16_t next_state_id = 0;
    T_transition *transition ;

    common_die_null(sm,-1,"null pointer" );
    common_die_null(sm->states_list   ,-2,"states_list    null pointer" );
    common_die_null(sm->state_current ,-5,"state_current  null pointer" );

    sm->nb_loop_in_curent_state++;

    /*
     * hack to avoid overload on long long states
     * as the handler do special action when the nb_loop_in_curent_state
     * is equal to 1. So we force value to an arbitrary one. 100 in this case
     */
    if(sm->nb_loop_in_curent_state == 0xFFFFFFFF)
        sm->nb_loop_in_curent_state = 100 ;

    /*
     * call each handler until a valid check popup
     */
    next_state_id = sm->state_current->ID;

    for (i = 0; i < STATEMACHINE_MAX_STATE_TRANSITIONS; i++) {
        transition = &sm->state_current->transitions_list[i];

        if (transition->handler == NULL)
            break;

        if (transition->handler(sm) == STATE_MACHINE_GOTO_NEXT_STATE) {
            if (next_state_id != transition->new_state)
                sm->nb_loop_in_curent_state = 0;

            next_state_id = transition->new_state;

            if (sm->debugging == true && sm->nb_loop_in_curent_state == 0)
                log_add("[%s] : TH[%02d]", sm->name, i);

            break;
        }
    }

    /*
     * change state if required
     */
    if (next_state_id != sm->state_current->ID) {
        // Search state corresponding to next_state_id
        for (i = 0 ;i < sm->number_of_states; i++) {
            if(sm->states_list[i].ID == next_state_id) {
                sm->state_current = &sm->states_list[i];
                if( sm->debugging == true)
                    log_add("[%s] ==> %s",sm->name,sm->state_current->name);
                break;
            }
        }
    }

    return 0;
}

int statemachine_wakeup(T_statemachine *sm)
{
    common_die_null(sm, -1, "sm null pointer");

    T_state  *old_current;
    int ret;
    int i = 0;

    /* run state machine several time until it gets a stable state */
    do {
        if (!sm->enable)
            return 0;

        old_current = sm->state_current;
        ret = statemachine_engine(sm);
        common_die_zero(ret,-2,"error handling state machine");
        i++;

        if (i > 10) {
            log_add("Warrning : loop in state machine !! %d->%d [%s]",
                    old_current->ID, sm->state_current->ID, sm->name);
            break;
        }
    } while((sm->loop_detection == true) && old_current != sm->state_current);

    return 0;
}

