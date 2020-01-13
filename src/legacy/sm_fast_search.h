/**
 * @file sm_fast_search.h
 */

#ifndef SM_FAST_SEARCH_H
#define SM_FAST_SEARCH_H


int sm_fast_search_init();
int sm_fast_search_reinit();
int sm_fast_search_wakeup();
int sm_fast_search_display_trace();

int is_sm_fast_search_nok();
int is_sm_fast_search_ok();
int is_sm_fast_search_in_error();

#endif /* SM_FAST_SEARCH_H */
