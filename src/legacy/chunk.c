#include <stdlib.h>

#include "log.h"
#include "chunk.h"
#include "filter.h"
#include "iq_fifo.h"
#include "signal_type.h"





T_chunk *chunk_alloc(E_signal_type type, uint32_t nb_samples, uint16_t samplecount)
{
    int i;

    T_chunk *chunk = malloc(sizeof(T_chunk));
    common_die_null(chunk, NULL, "failed to allocate chunk");

    int sample_size = signal_type_get_sample_size(type);
    common_die_zero(sample_size, NULL, "invalid sample size");

    uint8_t **buffer_array = malloc(samplecount * sizeof(uint8_t *));
    if (buffer_array == NULL) {
        free(chunk);
        common_die(NULL, "failed to allocate buffer array");
    }

    for (i = 0; i < samplecount; i++) {
        buffer_array[i] = malloc(nb_samples * sample_size);
        if (buffer_array[i] == NULL) {
            free(chunk);
            free(buffer_array);
            common_die(NULL, "failed to allocated buffer %d", i);
        }
    }

    chunk->type        = type;
    chunk->nb_samples  = nb_samples;
    chunk->samplecount = samplecount;
    chunk->buf         = buffer_array;

    return chunk;
}

int chunk_free(T_chunk *chunk)
{
    common_die_null(chunk, -1, "chunk null pointer");

    free(chunk->buf);
    free(chunk);

    return 0;
}

