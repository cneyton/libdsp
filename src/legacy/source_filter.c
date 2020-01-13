/**
 * @file buffer_src.c
 * @brief
 */

#include <stdint.h>

#include "filter.h"
#include "iq_fifo.h"
#include "source_filter.h"
#include "log.h"


struct {
    uint32_t repetition_count;
    uint16_t sample_count;
    uint8_t  nb_slots;
} g_sequence_config;


typedef struct {
    T_iq_fifo     *iq_fifo;
    /*T_oxy_fifo  *oxy_fifo;*/
    /*T_toco_fifo *toco_fifo;*/
    uint32_t      iq_chunk_size;
} T_source_filter_ctx;

static int _pipeline_run(T_filter *src)
{
    int ret;

    while (1) {
        ret = filter_pipeline_run(src->pipeline);
        common_die_zero(ret, -4, "failed to run pipeline");

        if (ret == 0)
            break;
    }

    return 0;
}

static int _activate(T_filter *filter)
{
    return 0;
}

int source_filter_init(T_filter *filter, E_source_type type)
{
    return 0;
}

int buffer_src_update_sequence_config(uint32_t repetition_count,
                                      uint8_t nb_slots,
                                      uint16_t samplecount)
{
    g_sequence_config.repetition_count = repetition_count;
    g_sequence_config.sample_count     = samplecount;
    g_sequence_config.nb_slots         = nb_slots;
    return 0;
}

int source_filter_push_frame_iq(T_filter *src, uint8_t *data)
{
    common_die_null(src, -1, "source filter null pointer");
    common_die_null(data, -2, "data null pointer");

    int ret;

    T_source_filter_ctx *ctx = (T_source_filter_ctx *)src->ctx;

    /*TODO: build frame*/
    T_frame frame;

    ret = iq_fifo_push_frame(ctx->iq_fifo, &frame);
    common_die_zero(ret, -3, "failed to push frame");

    if (iq_fifo_get_nb_samples(ctx->iq_fifo, 0) >= ctx->iq_chunk_size) {
        ret = _pipeline_run(src);
        common_die_zero(ret, -4, "failed to run pipeline");
    }

    return 0;
}

