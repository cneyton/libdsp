/**
 * @file iir_filter.c
 */

#include <string.h>
#include <stdlib.h>

#include "iir_filter.h"
#include "fifo.h"
#include "chunk.h"
#include "log.h"
#include "filter.h"

#define NB_INPUTS   1
#define NB_OUTPUTS  1

typedef struct {
    uint32_t len_a, len_b;   /* lenght of filter coefficients */
    double *a, *b;           /* filter coefficients */
    double *ic, *oc;         /* input and ouput cache */
} T_iir_ctx;

typedef struct {
    T_iir_ctx *iir_ctx_vector;
} T_iir_filter_ctx;

static int _activate_iir(T_iir_ctx *ctx, const double *src, double *dst, uint32_t nb_samples)
{
    common_die_null(ctx, -1, "ctx null pointer");
    common_die_null(src, -2, "src null pointer");
    common_die_null(dst, -3, "dst null pointer");

    unsigned n, i;
    double sample;

    for (n = 0; n < nb_samples; n++) {
        sample = 0.;

        memmove(&ctx->ic[1], &ctx->ic[0], (ctx->len_b - 1) * sizeof(*ctx->ic));
        memmove(&ctx->oc[1], &ctx->oc[0], (ctx->len_a - 1) * sizeof(*ctx->oc));
        ctx->ic[0] = src[n];
        for (i = 0; i < ctx->len_b; i++)
            sample += ctx->b[i] * ctx->ic[i];

        for (i = 1; i < ctx->len_a; i++)
            sample -= ctx->a[i] * ctx->oc[i];


        ctx->oc[0] = sample;
        dst[n] = sample;
    }

    return 0;
}

static int _activate(T_filter *filter)
{
    common_die_null(filter, -1, "filter null pointer");

    int ret;
    unsigned i;

    T_iir_filter_ctx *ctx = (T_iir_filter_ctx *)filter->ctx;
    common_die_null(ctx, -2, "ctx null pointer");

    if (fifo_is_empty(filter->inputs[0]->fifo)) {
        log_add("no chunk in input");
        return 0; /*What do if no inframe ?*/
    }

    T_chunk *in = filter_inlink_pop_chunk(filter->inputs[0]);
    common_die_null(in, -2, "failed to get chunk from input link");

    T_chunk *out = chunk_alloc(filter->outputs[0]->type,
                               filter->outputs[0]->nb_samples_in_chunk,
                               filter->samplecount);
    common_die_null(out, -3, "unable to get outlink chunk");

    for (i = 0; i < filter->samplecount; i++) {
        ret = _activate_iir(&ctx->iir_ctx_vector[i], (double *)in->buf[i], (double *)out->buf[i], in->nb_samples);
        common_die_zero(ret, -4, "failed to activate iir filter %d", i);
    }

    ret = filter_outlink_push_chunk(filter->outputs[0], out);
    common_die_zero(ret, -4, "failed to push chunk in output");

    return 0;
}

static int _iir_ctx_init(T_iir_ctx *ctx, double *b, double *a, uint32_t len_b, uint32_t len_a)
{
    common_die_null(ctx, -1, "ctx null pointer");
    common_die_null(b, -2, "b null pointer");
    common_die_null(a, -3, "a null pointer");

    if (!len_a || !len_b)
        common_die(-4, "one of the coefficients length is 0");

    ctx->ic = calloc(len_b + 1, sizeof(double));
    common_die_null(ctx->ic, -6, "input cache allocation failed");

    ctx->oc = calloc(len_a + 1, sizeof(double));
    if (ctx->oc == NULL) {
        free(ctx->ic);
        common_die(-7, "output cache allocation failed");
    }

    ctx->len_a = len_a;
    ctx->len_b = len_b;
    ctx->a     = a;
    ctx->b     = b;

    return 0;
}

static int _iir_ctx_uninit(T_iir_ctx *ctx)
{
    common_die_null(ctx, -1, "ctx null pointer");

    free(ctx->ic);
    free(ctx->oc);
    ctx->a = NULL;
    ctx->b = NULL;
    ctx->len_a = 0;
    ctx->len_b = 0;

    return 0;
}

static int _iir_ctx_reset(T_iir_ctx *ctx)
{
    common_die_null(ctx, -1, "ctx null pointer");

    double value = 0.0;
    unsigned i;


    for (i = 0; i < ctx->len_a; i++)
        memcpy(&ctx->ic[i], &value, sizeof(double));

    for (i = 0; i < ctx->len_b; i++)
        memcpy(&ctx->oc[i], &value, sizeof(double));

    return 0;
}

int iir_filter_init(T_filter *filter, uint16_t samplecount,
                    double *b, double *a, uint32_t len_b, uint32_t len_a)
{
    common_die_null(filter, -1, "filter null pointer");
    common_die_null(b, -2, "b null pointer");
    common_die_null(a, -3, "a null pointer");

    if (!len_a || !len_b)
        common_die(-4, "one of the coefficients length is 0");
    if (samplecount == 0)
        common_die(-5, "samplecount is 0");

    T_iir_filter_ctx *ctx = NULL;
    T_iir_ctx *iir_ctx_vector = NULL;
    int i = 0;
    int j, ret;

    ctx = malloc(sizeof(T_iir_filter_ctx));
    common_die_null(ctx, -5, "failed to allocate filter ctx");

    iir_ctx_vector = malloc(sizeof(T_iir_ctx) * samplecount);
    if (iir_ctx_vector == NULL) {
        free(ctx);
        common_die(-6, "failed to allocate iir ctx vector");
    }

    for (i = 0; i < samplecount; i++) {
        ret = _iir_ctx_init(&iir_ctx_vector[i], b, a, len_b, len_a);
        common_die_zero_goto(ret, ret, -7, error, "failed to allocate iir ctx %d", i);
    }

    ctx->iir_ctx_vector = iir_ctx_vector;
    filter->ctx = ctx;
    filter->nb_inputs   = NB_INPUTS;
    filter->nb_outputs  = NB_OUTPUTS;
    filter->samplecount = samplecount;
    filter->activate    = _activate;

    return 0;

error:
    for (j = 0; j < i; j++) {
        _iir_ctx_uninit(&iir_ctx_vector[j]);
    }
    free(iir_ctx_vector);
    free(ctx);
    return ret;
}

int iir_filter_uinit(T_filter *filter)
{
    common_die_null(filter, -1, "filter null pointer");

    int i, ret;

    T_iir_filter_ctx *ctx = (T_iir_filter_ctx *)filter->ctx;
    common_die_null(ctx, -2, "filter ctx null pointer");

    for (i = 0; i < filter->samplecount; i++) {
        ret = _iir_ctx_uninit(&ctx->iir_ctx_vector[i]);
        common_die_zero(ret, -3, "failed to uninit iir ctx %d", i);
    }

    free(ctx->iir_ctx_vector);
    free(ctx);

    return 0;
}

int iir_filter_reset(T_filter *filter)
{
    common_die_null(filter, -1, "filter null pointer");

    unsigned i;
    int ret;

    T_iir_filter_ctx *ctx = (T_iir_filter_ctx *)filter->ctx;
    common_die_null(ctx, -2, "filter ctx null pointer");

    for (i = 0; i < filter->samplecount; i++) {
        ret = _iir_ctx_reset(&ctx->iir_ctx_vector[i]);
        common_die_zero(ret, -3, "failed to reset ctx %d", i);
    }

    return 0;
}

