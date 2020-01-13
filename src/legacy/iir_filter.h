/**
 * @file iir_filter.h
 */

#ifndef _IIR_FILTER_
#define _IIR_FILTER_

#include <stdint.h>
#include <float.h>

#include "filter.h"


int iir_filter_init(T_filter *filter, uint16_t samplecount,
                    double *b, double *a, uint32_t len_b, uint32_t len_a);
int iir_filter_uinit(T_filter *filter);
int iir_filter_reset(T_filter *filter);

#endif
