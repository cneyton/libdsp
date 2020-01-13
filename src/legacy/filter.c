#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "filter.h"
#include "iq_fifo.h"
#include "chunk.h"


int filter_init(T_filter *filter, char *name)
{
    int i;

    strncpy(filter->name, name, FILTER_NAME_MAX_LENGTH);
    filter->name[FILTER_NAME_MAX_LENGTH-1]='\0';
    filter->nb_inputs = 0;
    filter->nb_outputs = 0;
    filter->samplecount = 0;
    filter->ctx = NULL;
    filter->ready = false;
    filter->activate = NULL;

    for (i = 0; i < FILTER_NB_MAX_LINK; i++) {
        filter->inputs[i] = NULL;
        filter->outputs[i] = NULL;
    }

    return 0;
}

int link_init(T_link *link)
{
    link->src  = NULL;
    link->dst  = NULL;
    link->fifo = NULL;
    link->sample_rate = 1;
    link->timestamp   = 0;
    link->nb_samples_in_chunk = 0;

    return 0;
}

int filter_link(T_link *link, T_filter *src, T_filter *dst,
                uint32_t nb_chunks, E_signal_type type, uint32_t nb_samples_in_chunk)
{
    common_die_null(link, -1 ,"link null pointer");
    common_die_null(src, -2 ,"src filter null pointer");
    common_die_null(dst, -3 ,"dst filter null pointer");

    int i;

    if (src->samplecount != dst->samplecount)
        common_die(-4, "input and output format of the link are different");

    link->fifo = fifo_alloc(nb_chunks, sizeof(T_chunk_fifo_elt));
    common_die_null(link->fifo, -4, "failed to allocate fifo for link");

    link->dst = dst;
    link->src = src;
    link->type = type;
    link->nb_samples_in_chunk = nb_samples_in_chunk;
    link->samplecount = src->samplecount;

    for (i = 0; i < src->nb_outputs; i++) {
        if (src->outputs[i] == NULL) {
            src->outputs[i] = link;
            break;
        }
    }

    for (i = 0; i < dst->nb_inputs; i++) {
        if (dst->inputs[i] == NULL) {
            dst->inputs[i] = link;
            break;
        }
    }

    return 0;
}

T_chunk *filter_inlink_pop_chunk(T_link *link)
{
    common_die_null(link, NULL, "link null pointer");

    int ret;
    T_chunk_fifo_elt elt;

    ret = fifo_pop(link->fifo, &elt);
    common_die_zero(ret, NULL, "failed to pop chunk from link fifo");

    if (ret == 0)
        return NULL;

    return elt.chunk;
}

int filter_outlink_push_chunk(T_link *link, T_chunk *chunk)
{
    common_die_null(link, -1, "link null pointer");
    common_die_null(chunk, -2, "chunk null pointer");

    int ret;
    T_chunk_fifo_elt elt;

    elt.chunk = chunk;

    ret = fifo_push(link->fifo, &elt);
    common_die_zero(ret, -3, "failed to push chunk in link fifo");

    link->dst->ready = 1;

    return 0;
}

int filter_pipeline_init(T_pipeline *pipeline)
{
    common_die_null(pipeline, -1, "pipeline pointer null");

    unsigned i;

    for (i = 0; i < FILTER_NB_MAX_FILTERS; i++) {
        pipeline->filters[i] = NULL;
    }

    pipeline->nb_filters = 0;

    return 0;
}

int filter_pipeline_add_filter(T_pipeline *pipeline, T_filter *filter)
{
    common_die_null(pipeline, -1, "pipeline null pointer");
    common_die_null(filter,   -2, "filter null pointer");

    pipeline->filters[pipeline->nb_filters] = filter;
    pipeline->nb_filters++;

    filter->pipeline = pipeline;

    return 0;
}

/**
* @brief Run the processing pipeline once
*
* @param pipeline : pointer to the pipeline
*
* @return > 0 : a filter was activated
*           0 : no filter activated
*         < 0 : error
*/
int filter_pipeline_run(T_pipeline *pipeline)
{
    common_die_null(pipeline, -1, "pipeline null pointer");

    int      ret;
    unsigned i;
    T_filter *filter;

    for (i = 0; i < pipeline->nb_filters; i++) {
        filter = pipeline->filters[i];
        common_die_null(filter, -2, "filter null pointer");
        if (filter->ready && filter->activate != NULL) {
            ret = filter->activate(filter);
            common_die_zero(ret, -2, "failed to activate filter %s", filter->name);
            filter->ready = false;
            return 1;
        }
    }

    return 0;
}

