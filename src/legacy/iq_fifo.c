#include <stdint.h>
#include <stdlib.h>

#include "fifo.h"
#include "iq_fifo.h"
#include "log.h"


T_iq_fifo *iq_fifo_alloc(uint16_t samplecount, uint32_t nb_tot_frames)
{
    if (samplecount == 0)
        common_die(NULL, "samplecount is 0");

    if (nb_tot_frames == 0)
        common_die(NULL, "total number of samples is 0");

    int ret;
    uint16_t i;

    T_iq_fifo *iq_fifo = malloc(sizeof(T_iq_fifo));
    common_die_null(iq_fifo, NULL, "iq_fifo allocation failed");

    T_fifo **fifo_vector = malloc(samplecount * sizeof(T_fifo*));
    if (fifo_vector == NULL) {
        free(iq_fifo);
        common_die(NULL, "allocation of fifo vector failed")
    }

    for (i = 0; i < samplecount; i++) {
        fifo_vector[i] = fifo_alloc(nb_tot_frames, sizeof(T_iq_sample));
        if (fifo_vector[i] == NULL) {
            free(fifo_vector);
            free(iq_fifo);
            common_die(NULL, "allocation of a fifo in fifo vector failed");
        }
    }

    iq_fifo->samplecount = samplecount;
    iq_fifo->nb_tot_frames = nb_tot_frames;
    iq_fifo->nb_frames = 0;
    iq_fifo->fifo_vector = fifo_vector;

    return iq_fifo;
}

int iq_fifo_free(T_iq_fifo *iq_fifo)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");

    uint16_t i;

    for (i = 0; i < iq_fifo->samplecount; i++) {
        fifo_free(iq_fifo->fifo_vector[i]);
    }
    free(iq_fifo->fifo_vector);
    free(iq_fifo);

    return 0;
}

int iq_fifo_push_sample(T_iq_fifo *iq_fifo, T_iq_sample *sample, uint16_t sample_nb)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");
    common_die_null(sample,      -2, "sample null pointer");

    if (sample_nb >= iq_fifo->samplecount)
        common_die(-3, "sample_nb >= samplecount (%d >= %d)", sample_nb, iq_fifo->samplecount);

    int ret;

    ret = fifo_push(iq_fifo->fifo_vector[sample_nb], sample);
    common_die_zero(ret, -4, "failed to push sample number %d", sample_nb);

    return 0;
}

int iq_fifo_push_slot(T_iq_fifo *iq_fifo, T_slot *slot)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");
    common_die_null(slot,        -2, "slot null pointer");

    uint16_t i;
    int ret;

    for (i = 0; i < slot->samplecount; i++) {
        ret = iq_fifo_push_sample(iq_fifo, &slot->samples[i], i);
        common_die_zero(ret, -3, "failed to push sample for slot %d", slot->slot_nb);
    }

    return 0;
}

int iq_fifo_push_frame(T_iq_fifo *iq_fifo, T_frame *frame)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");
    common_die_null(frame,       -2, "frame null pointer");

    uint16_t i;
    int ret;

    for (i = 0; i < frame->nb_slots; i++) {
        ret = iq_fifo_push_slot(iq_fifo, &frame->slots[i]);
        common_die_zero(ret, -3, "failed to push slot for frame number %d", frame->id);
    }

    return 0;
}

int iq_fifo_pop_sample(T_iq_fifo *iq_fifo, T_iq_sample *sample, uint16_t sample_nb)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");
    common_die_null(sample,      -2, "sample null pointer");

    if (sample_nb >= iq_fifo->samplecount)
        common_die(-3, "sample_nb >= samplecount (%d >= %d)", sample_nb, iq_fifo->samplecount);

    int ret;

    ret = fifo_pop(iq_fifo->fifo_vector[sample_nb], sample);
    common_die_zero(ret, -4, "failed to pop sample number %d", sample_nb);

    return 0;
}

int iq_fifo_flush(T_iq_fifo *iq_fifo)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");

    uint16_t i;
    int ret;

    for (i = 0; i < iq_fifo->samplecount; i++) {
        ret = fifo_flush(iq_fifo->fifo_vector[i]);
        common_die_zero(ret, -2, "failed to flush fifo %d", i);
    }

    iq_fifo->nb_frames = 0;

    return 0;
}

int iq_fifo_is_full(T_iq_fifo *iq_fifo, uint16_t sample_nb)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");

    int ret;

    ret = fifo_is_full(iq_fifo->fifo_vector[sample_nb]);
    common_die_zero(ret, -2, "failed to check if fifo %d is full", sample_nb);

    return ret;
}

int iq_fifo_is_empty(T_iq_fifo *iq_fifo, uint16_t sample_nb)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");

    int ret;

    ret = fifo_is_empty(iq_fifo->fifo_vector[sample_nb]);
    common_die_zero(ret, -2, "failed to check if fifo %d is empty", sample_nb);

    return ret;
}

int iq_fifo_get_nb_samples(T_iq_fifo *iq_fifo, uint16_t sample_nb)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");

    int ret;

    ret = fifo_get_nb_elt(iq_fifo->fifo_vector[sample_nb]);
    common_die_zero(ret, -2, "failed to get number of elt of fifo %d", sample_nb);

    return ret;
}

int iq_fifo_get_nb_free_samples(T_iq_fifo *iq_fifo, uint16_t sample_nb)
{
    common_die_null(iq_fifo, -1, "iq_fifo null pointer");

    int ret;

    ret = fifo_get_nb_free_elt(iq_fifo->fifo_vector[sample_nb]);
    common_die_zero(ret, -2, "failed to get number of free elt in fifo %d", sample_nb);

    return ret;
}
