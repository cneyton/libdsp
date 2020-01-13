/**
 * @file iq_fifo.h
 */

#ifndef _IQ_FIFO_H_
#define _IQ_FIFO_H_

#include <stdint.h>

#include "fifo.h"
#include "signal_type.h"

typedef struct {
    T_fifo   **fifo_vector;
    uint16_t samplecount;
    uint32_t nb_tot_frames;
    uint32_t nb_frames;
} T_iq_fifo;

typedef struct {
    uint16_t    burst;
    uint16_t    channel;
    uint16_t    tus;
    uint16_t    slot_nb;
    uint16_t    samplecount;
    T_iq_sample *samples;
} T_slot;

typedef struct{
    uint8_t  nb_slots;
    uint32_t id;
    T_slot  *slots;
} T_frame;

T_iq_fifo *iq_fifo_alloc(uint16_t samplecount, uint32_t nb_tot_frames);
int iq_fifo_free (T_iq_fifo *iq_fifo);

int iq_fifo_push_sample(T_iq_fifo *iq_fifo, T_iq_sample *sample, uint16_t sample_nb);
int iq_fifo_push_slot  (T_iq_fifo *iq_fifo, T_slot *slot);
int iq_fifo_push_frame (T_iq_fifo *iq_fifo, T_frame *frame);

int iq_fifo_pop_sample(T_iq_fifo *iq_fifo, T_iq_sample *sample, uint16_t sample_nb);

int iq_fifo_flush(T_iq_fifo *iq_fifo);

int iq_fifo_is_full (T_iq_fifo *iq_fifo, uint16_t sample_nb);
int iq_fifo_is_empty(T_iq_fifo *iq_fifo, uint16_t sample_nb);

int iq_fifo_get_nb_samples     (T_iq_fifo *iq_fifo, uint16_t sample_nb);
int iq_fifo_get_nb_free_samples(T_iq_fifo *iq_fifo, uint16_t sample_nb);

#endif

