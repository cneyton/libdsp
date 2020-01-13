/**
 * @file signal_type.h
 */

#ifndef _SIGNAL_TYPE_H
#define _SIGNAL_TYPE_H

#include <stdint.h>
#include <stdlib.h>

#include "log.h"

typedef enum {
    SIGNAL_TYPE_UNKNOWN,
    SIGNAL_TYPE_IQ_S32,
    SIGNAL_TYPE_IQ_DBL,
    SIGNAL_TYPE_RF,
    SIGNAL_TYPE_OXY_S32,
    SIGNAL_TYPE_OXY_DBL,
    SIGNAL_TYPE_TOCO
} E_signal_type;

typedef struct {
    double i;
    double q;
} T_iq_sample;

typedef struct {
    int16_t i;
    int16_t q;
} T_iq_raw_sample;

#define IQ_SAMPLE(name, type)  \
    typedef struct {           \
        type i;                \
        type q;                \
    } T_iq_sample_##name;

IQ_SAMPLE(s32, int32_t);
IQ_SAMPLE(flt, float);
IQ_SAMPLE(dbl, double);

#define OXY_SAMPLE(name, type) \
    typedef type T_oxy_sample_##name;

OXY_SAMPLE(s32, int32_t);
OXY_SAMPLE(flt, float);
OXY_SAMPLE(dbl, double);

static int signal_type_get_sample_size(E_signal_type type)
{
    size_t sample_size;

    switch (type) {
    case SIGNAL_TYPE_IQ_S32:   sample_size = sizeof(T_iq_sample_s32);     break;
    case SIGNAL_TYPE_IQ_DBL:   sample_size = sizeof(T_iq_sample_dbl);     break;
    case SIGNAL_TYPE_RF:
    case SIGNAL_TYPE_OXY_S32:  sample_size = sizeof(T_oxy_sample_s32);    break;
    case SIGNAL_TYPE_OXY_DBL:  sample_size = sizeof(T_oxy_sample_dbl);    break;
    case SIGNAL_TYPE_TOCO:     sample_size = sizeof(uint8_t);             break;
    default:                   common_die(-2, "invalid signal type");     break;
    }

    return (int)sample_size;
}


int signal_type_get_sample_size(E_signal_type type);

#endif
