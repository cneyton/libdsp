/**
 * @file filter.h
 */

#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdint.h>

#include "fifo.h"
#include "chunk.h"
#include "signal_type.h"


#define FILTER_NAME_MAX_LENGTH 100
#define FILTER_NB_MAX_LINK     10
#define FILTER_NB_MAX_FILTERS  20

#define LINK_INITIALIZER                \
    (T_link) {                          \
        .src  = NULL,                   \
        .dst  = NULL,                   \
        .fifo = NULL,                   \
        .sample_rate = 1,               \
        .timestamp   = 0,               \
        .nb_samples_in_chunk = 0,       \
        .samplecount = 0,               \
        .type = SIGNAL_TYPE_UNKNOWN     \
    }

#define FILTER_INITIALIZER              \
    (T_filter) {                        \
        .name = "",                     \
        .nb_inputs = 0,                 \
        .nb_outputs = 0,                \
        .samplecount = 0,               \
        .inputs = {NULL},               \
        .outputs = {NULL},              \
        .ctx = NULL,                    \
        .ready = false,                 \
        .activate = NULL,               \
        .pipeline = NULL                \
    }

#define PIPELINE_INITIALIZER            \
    (T_pipeline) {                      \
        .filters = {NULL},              \
        .nb_filters = 0                 \
    }

typedef struct T_filter   T_filter;
typedef struct T_pipeline T_pipeline;

typedef struct {
    T_filter  *src;
    T_filter  *dst;
    T_fifo    *fifo;
    uint32_t  sample_rate;
    uint32_t  timestamp;
    uint32_t  nb_samples_in_chunk;
    uint16_t  samplecount;
    E_signal_type type;
} T_link;

struct T_filter {
    char         name[FILTER_NAME_MAX_LENGTH];
    uint32_t     nb_inputs;
    uint32_t     nb_outputs;
    uint16_t     samplecount;
    T_link     * inputs[FILTER_NB_MAX_LINK];
    T_link     * outputs[FILTER_NB_MAX_LINK];
    T_pipeline * pipeline;
    void       * ctx;
    int          ready;
    int        (*activate)(T_filter *);
};

struct T_pipeline {
    T_filter * filters[FILTER_NB_MAX_FILTERS];
    uint32_t   nb_filters;
};

int filter_init(T_filter *filter, char *name);

int filter_link(T_link *link, T_filter *src, T_filter *dst,
                uint32_t nb_chunks, E_signal_type type, uint32_t nb_samples_in_chunk);

T_chunk *filter_inlink_pop_chunk(T_link *link);
int filter_outlink_push_chunk(T_link *link, T_chunk *chunk);

int filter_pipeline_run       (T_pipeline *pipeline);
int filter_pipeline_add_filter(T_pipeline *pipeline, T_filter *filter);

#endif
