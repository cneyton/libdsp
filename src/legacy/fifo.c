/**
 * @file fifo.c
 * @brief handdle simple fifo
 */

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "fifo.h"


/**
* @brief Allocate a fifo with nb_tot_elt of size
*
* @param f : pointer to the pointer of the fifo
* @param nb_tot_elt : total number of element in the fifo
* @param size_elt : size of one element of the fifo
*
* @return < 0 on error,
*         >= 0 : number of free element in the fifo
*/
T_fifo *fifo_alloc(uint32_t nb_tot_elt, uint32_t size_elt)
{
    if (!size_elt)
        common_die(NULL, "fifo elt size is 0");

    if (nb_tot_elt >= SIZE_MAX/size_elt)
        common_die(NULL, "fifo nb of elt too large");

    uint8_t *buffer = malloc((size_t)(nb_tot_elt) * (size_t)(size_elt));
    common_die_null(buffer, NULL, "fifo buffer allocation failed");

    T_fifo *f = malloc(sizeof(T_fifo));
    if (f == NULL) {
        free(buffer);
        common_die(NULL, "fifo allocation failed")
    }

    f->buffer = buffer;
    f->head   = buffer;
    f->queue  = buffer;
    f->size_elt    = size_elt;
    f->nb_tot_elt  = nb_tot_elt;
    f->nb_elt      = 0;
    f->nb_free_elt = nb_tot_elt;
    f->end = buffer + (nb_tot_elt - 1) * size_elt;

    return f;
}


T_fifo *fifo_allocz(uint32_t nb_tot_elt, uint32_t size_elt)
{
    T_fifo *f = fifo_alloc(nb_tot_elt, size_elt);
    common_die_null(f, NULL, "fifo allocation failed");

    memset(f->buffer, 0, f->end - f->buffer);

    return f;
}


/**
* @brief Free an allocated fifo
*
* @param f : initialized fifo
*
* @return <0 on error, 0 on success
*/
int fifo_free(T_fifo *f)
{
    common_die_null(f, -1, "fifo null pointer");
    free(f->buffer);
    free(f);

    return 0;
}


/**
* @brief Push an element to the fifo
*
* @param f : initialized fifo
* @param elt : element to copy to the fifo
*
* @return <0 on error, 0 on success
*/
int fifo_push(T_fifo *f, void *elt)
{
    common_die_null(f, -1, "fifo null pointer");
    common_die_null(elt, -2, "elt null pointer");

    if (f->nb_free_elt == 0) {
        log_add("fifo full");
        return -3;
    }

    memcpy(f->queue, elt, f->size_elt);

    f->nb_elt = f->nb_elt + 1;
    f->nb_free_elt = f->nb_free_elt - 1;
    if (f->queue != f->end)
        f->queue = f->queue + f->size_elt;
    else
        f->queue = f->buffer;

    return 0;
}


/**
* @brief Pop an element from the fifo
*
* @param f : an initialized fifo
* @param elt : space where the head fifo element will be copied to
*
* @return < 0 on error,
*           0 if fifo is empty nothing has been copied,
*           1 if sucess
*/
int fifo_pop(T_fifo * f, void *elt)
{
    common_die_null(f, -1, "fifo null pointer");
    common_die_null(elt, -2, "elt null pointer");

    if (f->nb_elt == 0) {
        return 0;
    }

    memcpy(elt, f->head, f->size_elt);

    f->nb_elt = f->nb_elt - 1;
    f->nb_free_elt = f->nb_free_elt + 1;
    if (f->head != f->end)
        f->head = f->head + f->size_elt;
    else
        f->head = f->buffer;

    return 1;
}


/**
* @brief Peak at element at pos latest place in the fifo
*
* @param f : an initialized fifo
* @param elt : pointer to the elt in the fifo
* @param pos : position from the queue of the fifo
*
* @return < 0 on error, 0 on success
*/
int fifo_peek(T_fifo *f,  void const *elt, uint32_t pos)
{
    common_die_null(f, -1, "fifo null pointer");
    common_die_null(elt, -2, "elt null pointer");

    if (pos >= f->nb_elt)
        common_die(-3, "can't peak at pos %d, nb_elt = %d", pos, f->nb_elt);

    uint8_t *ptr = f->queue + (pos * f->size_elt);

    if (ptr > f->end)
        ptr = f->buffer + (ptr - f->end);

    elt = ptr;

    return 0;
}


/**
* @brief Flush all element of the fifo
*
* @param f : an initialized fifo
*
* @return < 0 on error, 0 on success
*/
int fifo_flush(T_fifo *f)
{
    common_die_null(f, -1, "fifo null pointer");

    f->nb_elt      = 0;
    f->nb_free_elt = f->nb_tot_elt;
    f->head        = f->buffer;
    f->queue       = f->buffer;

    return 0;
}


/**
* @brief Tell if a fifo is full
*
* @param f : an initialized fifo
*
* @return < 0 on error,
*           0 : fifo not full
*           1 : fifo full
*/
int fifo_is_full(T_fifo *f)
{
    common_die_null(f, -1, "fifo null pointer");

    return (f->nb_free_elt == 0);
}


/**
* @brief Tell if a fifo is empty
*
* @param f : an initialized fifo
*
* @return < 0 on error,
*           0 : fifo not empty
*           1 : fifo empty
*/
int fifo_is_empty(T_fifo * f)
{
    common_die_null(f, -1, "fifo null pointer");

    return (f->nb_elt == 0);
}


/**
* @brief Tell the number of used element in a fifo
*
* @param f : an initialized fifo
*
* @return < 0 on error,
*         >= 0 : number of used element in the fifo
*/
int fifo_get_nb_elt(T_fifo * f)
{
    common_die_null(f, -1, "fifo null pointer");

    return f->nb_elt;
}


/**
* @brief Tell the number of free element in a fifo
*
* @param f : an initialized fifo
*
* @return < 0 on error,
*         >= 0 : number of free element in the fifo
*/
int fifo_get_nb_free_elt(T_fifo *f)
{
    common_die_null(f, -1, "fifo null pointer");

    return f->nb_free_elt;
}

