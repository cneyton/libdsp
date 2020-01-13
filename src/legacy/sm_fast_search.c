#include "sm_fast_search.h"
#include "statemachine.h"
#include "log.h"


enum {
    STATE_FAST_SEARCH_INIT,
    STATE_FAST_SEARCH_CENTER,
    STATE_FAST_SEARCH_LEFT,
    STATE_FAST_SEARCH_RIGHT,
    STATE_FAST_SEARCH_TOP,
    STATE_FAST_SEARCH_BOT,
    STATE_FAST_SEARCH_ERROR,
    STATE_FAST_SEARCH_OK,
    STATE_FAST_SEARCH_NOK,
};


static int _check_heart_found(T_statemachine *sm);
static int _check_error(T_statemachine *sm);
static int _check_init_done(T_statemachine *sm);
static int _check_true(T_statemachine *sm);


static int _handler_state_init(T_statemachine *sm);
static int _handler_state_center(T_statemachine *sm);
static int _handler_state_left(T_statemachine *sm);
static int _handler_state_right(T_statemachine *sm);
static int _handler_state_top(T_statemachine *sm);
static int _handler_state_bot(T_statemachine *sm);
static int _handler_state_error(T_statemachine *sm);
static int _handler_state_ok(T_statemachine *sm);
static int _handler_state_nok(T_statemachine *sm);


static T_state g_sm_fast_search_states[] = {
    STATE_BEGIN(STATE_FAST_SEARCH_INIT)
    { _handler_state_init,          STATE_FAST_SEARCH_INIT},
    { _check_error,                 STATE_FAST_SEARCH_ERROR},
    { _check_init_done,             STATE_FAST_SEARCH_CENTER},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_CENTER)
    { _handler_state_center,        STATE_FAST_SEARCH_CENTER},
    { _check_error,                 STATE_FAST_SEARCH_ERROR},
    { _check_heart_found,           STATE_FAST_SEARCH_OK},
    { _check_true,                  STATE_FAST_SEARCH_LEFT},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_LEFT)
    { _handler_state_left,          STATE_FAST_SEARCH_LEFT},
    { _check_error,                 STATE_FAST_SEARCH_ERROR},
    { _check_heart_found,           STATE_FAST_SEARCH_OK},
    { _check_true,                  STATE_FAST_SEARCH_RIGHT},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_RIGHT)
    { _handler_state_right,         STATE_FAST_SEARCH_RIGHT},
    { _check_error,                 STATE_FAST_SEARCH_ERROR},
    { _check_heart_found,           STATE_FAST_SEARCH_OK},
    { _check_true,                  STATE_FAST_SEARCH_TOP},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_TOP)
    { _handler_state_top,           STATE_FAST_SEARCH_TOP},
    { _check_error,                 STATE_FAST_SEARCH_ERROR},
    { _check_heart_found,           STATE_FAST_SEARCH_OK},
    { _check_true,                  STATE_FAST_SEARCH_BOT},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_BOT)
    { _handler_state_bot,           STATE_FAST_SEARCH_BOT},
    { _check_error,                 STATE_FAST_SEARCH_ERROR},
    { _check_heart_found,           STATE_FAST_SEARCH_OK},
    { _check_true,                  STATE_FAST_SEARCH_NOK},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_NOK)
    { _handler_state_nok,           STATE_FAST_SEARCH_NOK},
    STATE_END

    STATE_BEGIN(STATE_FAST_SEARCH_OK)
    { _handler_state_ok,           STATE_FAST_SEARCH_OK},
    STATE_END
};

struct {
    T_statemachine sm;
} g_sm_fast_search;


static int _check_heart_found(T_statemachine *sm)
{
    return 0;
}

static int _check_error(T_statemachine *sm)
{
    return 0;
}

static int _check_init_done(T_statemachine *sm)
{
    return 1;
}

static int _check_true(T_statemachine *sm)
{
    return 1;
}

static int _handler_state_init(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_center(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_left(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_right(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_top(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_bot(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_error(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_ok(T_statemachine *sm)
{
    return 0;
}

static int _handler_state_nok(T_statemachine *sm)
{
    return 0;
}

int sm_fast_search_init()
{
    int ret;

    ret = statemachine_init(&g_sm_fast_search.sm, g_sm_fast_search_states,
                            sizeof(g_sm_fast_search_states) / sizeof(T_state),
                            "sm_fast_search", STATE_FAST_SEARCH_INIT, true);
    common_die_zero(ret, -1, "sm_fast_search init failed");

    return 0;
}

int sm_fast_search_wakeup()
{
    int ret;

    ret = statemachine_wakeup(&g_sm_fast_search.sm);
    common_die_zero(ret, -1, "sm_fast_search wakeup failed");

    return 0;
}

int sm_fast_search_reinit()
{
    int ret;

    ret = statemachine_reinit(&g_sm_fast_search.sm);
    common_die_zero(ret, -1, "sm_fast_search reinit failed");

    return 0;
}

int sm_fast_search_display_trace()
{
    int ret;

    ret = statemachine_display_trace(&g_sm_fast_search.sm);
    common_die_zero(ret, -1, "sm_fast_search display trace failed");

    return 0;
}

int is_sm_fast_search_in_error()
{
    if (statemachine_get_curstate(&g_sm_fast_search.sm) == STATE_FAST_SEARCH_ERROR) {
        return 1;
    }
    return 0;
}

int is_sm_fast_search_ok()
{
    if (statemachine_get_curstate(&g_sm_fast_search.sm) == STATE_FAST_SEARCH_OK) {
        return 1;
    }
    return 0;
}

int is_sm_fast_search_nok()
{
    if (statemachine_get_curstate(&g_sm_fast_search.sm) == STATE_FAST_SEARCH_NOK) {
        return 1;
    }
    return 0;
}
