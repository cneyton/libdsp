/**
 * @file chunk.h
 */

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#include "signal_type.h"
#include "list.h"


typedef struct {
    uint32_t       nb_samples;
    uint16_t       samplecount;
    E_signal_type  type;
    uint8_t        **buf;       /* Per samplecount data buffer */
} T_chunk;

typedef struct {
    T_chunk          *chunk;
} T_chunk_fifo_elt;

T_chunk * chunk_alloc(E_signal_type type, uint32_t nb_samples, uint16_t samplecount);
int       chunk_free(T_chunk *chunk);

#endif
