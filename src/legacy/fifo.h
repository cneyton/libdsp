/**
 * @file fifo.h
 * @brief handle simple fifo
 */

#ifndef _FIFO_H_
#define _FIFO_H_

#include <stdint.h>

typedef struct {
	uint8_t *buffer;
	uint8_t *head;
	uint8_t *queue;
	uint8_t *end;
	uint32_t size_elt;
        uint32_t nb_tot_elt;
	uint32_t nb_elt;
	uint32_t nb_free_elt;
} T_fifo;

T_fifo *fifo_alloc       (uint32_t nb_tot_elt, uint32_t size_elt);
T_fifo *fifo_allocz      (uint32_t nb_tot_elt, uint32_t size_elt);
int fifo_free            (T_fifo *f);
int fifo_push            (T_fifo *f, void *elt);
int fifo_pop             (T_fifo *f, void *elt);
int fifo_peek            (T_fifo *f, void const *elt, uint32_t pos);
int fifo_flush           (T_fifo *f);
int fifo_is_full         (T_fifo *f);
int fifo_is_empty        (T_fifo *f);
int fifo_get_nb_elt      (T_fifo *f);
int fifo_get_nb_free_elt (T_fifo *f);

#endif

