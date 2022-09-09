#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <sys/time.h>

#include "bb.h"

/** @brief Max size for the Cl set */
#define CL_SIZE_MAX 2
/** @brief Value for a { } (empty) Cl set */
#define CL_EMPTY NULL
/** @brief Value for a { 0 } Cl set */
#define CL_ZERO (&_CL_ZERO)
static bool _CL_ZERO[CL_SIZE_MAX] = { false };
/** @brief Value for a { 1, 0 } Cl set */
#define CL_ONE_OR_ZERO (&_CL_ONE_OR_ZERO)
static bool _CL_ONE_OR_ZERO[CL_SIZE_MAX] = { true, false };

/** @brief "Global" references structure */
struct _bb_ctx {
    /** bounding function */
    bb_fn B;
    /** current optimal profit */
    unsigned opt_P;
    /** current optimal solution */
    unsigned *opt_X;
    /** cast actors */
    struct bb_actor *E;
    /** not cast actors */
    struct bb_actor *F;
    /** current feasible solution */
    bool *X;
    /** helper lens for counting distinct groups */
    bool *lens_S;
    /** @ref CL_EMPTY, @ref CL_ONE_OR_ZERO or @ref CL_ZERO */
    bool (*Cl)[CL_SIZE_MAX];
    /** total of visited nodes */
    unsigned visited_nodes;
    /** total optimality cuts */
    unsigned optimality_cuts;
    /** total of feasibility cuts */
    unsigned feasibility_cuts;
};

/**
 * @brief Obtains count of casted actors
 *
 * @param in parsed input
 * @param ctx "global" references
 * @return total count of casted actors
 */
static size_t
_bb_actors_count(const struct bb_input *in, const struct _bb_ctx *ctx)
{
    size_t total_a = 0;
    for (size_t i = 0; i < in->m; ++i)
        total_a += ctx->X[i];
    return total_a;
}

/**
 * @brief Obtains count of groups covered by casted actors
 *
 * @param in parsed input
 * @param ctx "global" references
 * @param sub_S optional groups subset to be added with groups
 *      covered by casted actors
 * @param sub_Ss amount of elements in the groups subsets
 * @return total count of casted actors
 */
static size_t
_bb_group_count(const struct bb_input *in,
                struct _bb_ctx *ctx,
                const unsigned sub_S[],
                size_t sub_Ss)
{
    size_t total_s = 0;
    if (sub_S != NULL) {
        for (size_t i = 0; i < sub_Ss; ++i) {
            if (ctx->lens_S[i] == true) continue;

            ctx->lens_S[sub_S[i] - 1] = true;
            ++total_s;
        }
    }
    for (size_t i = 0; i < in->m; ++i) {
        if (ctx->X[i] == false) continue;

        for (size_t j = 0; j < in->A[i].s; ++j) {
            if (ctx->lens_S[in->A[i].sub_S[j] - 1] == true) continue;

            ctx->lens_S[in->A[i].sub_S[j] - 1] = true;
            ++total_s;
        }
    }
    for (size_t i = 0; i < in->l; ++i)
        ctx->lens_S[i] = 0;
    return total_s;
}

/**
 * @brief Total profit for current feasible solution
 *
 * @param in parsed input
 * @param ctx "global" references
 * @return total profit
 */
static unsigned
_bb_profit(const struct bb_input *in, const struct _bb_ctx *ctx)
{
    unsigned P = 0;
    for (size_t i = 0; i < in->m; ++i)
        P += ctx->X[i] * in->A[i].c;
    return P;
}

/**
 * @brief Compute the Cl set for the current iteration
 *
 * @param in data parsed at input
 * @param ctx "global" references
 * @param l length of current feasible solution
 * @param sub_Am cast actors count
 */
static void
_bb_Cl_compute(struct bb_input *in,
               struct _bb_ctx *ctx,
               size_t l,
               size_t sub_Am)
{
    if (l == in->m) {
        ctx->Cl = CL_EMPTY;
        return;
    }

    if (in->has_feasibility_cuts) {
        ++ctx->feasibility_cuts;
        if (sub_Am == in->n) {
            ctx->Cl = CL_ZERO;
            return;
        }
        if (in->l > _bb_group_count(in, ctx, in->A[l].sub_S, in->A[l].s)) {
            ctx->Cl = CL_ONE_OR_ZERO;
            return;
        }
    }

    ctx->Cl = CL_ONE_OR_ZERO;
}

/**
 * @brief Solve the casting problem from `README.pdf` with Branch and Bound
 *      method
 *
 * @param in data parsed at input
 * @param ctx "global" references
 * @param l index of current x node
 */
static void
_bb_solve(struct bb_input *in, struct _bb_ctx *ctx, size_t l)
{
    const size_t sub_Am = _bb_actors_count(in, ctx);
    unsigned nextbound[CL_SIZE_MAX];
    bool nextchoice[CL_SIZE_MAX];
    size_t count = 0;

    ++ctx->visited_nodes;

    if (sub_Am == in->n && _bb_group_count(in, ctx, NULL, 0) == in->l) {
        const unsigned P = _bb_profit(in, ctx);
        if (P < ctx->opt_P) {
            ctx->opt_P = P;
            for (size_t i = 0; i < in->m; ++i)
                ctx->opt_X[i] = ctx->X[i];
        }
    }

    _bb_Cl_compute(in, ctx, l, sub_Am);
    if (ctx->Cl != CL_EMPTY) {
        /* E = currently cast actors; F = not yet cast actors */
        size_t En = 0, Fn = 0;
        for (size_t i = 0; i < in->m; ++i) {
            if (ctx->X[i])
                ctx->E[En++] = in->A[i];
            else
                ctx->F[Fn++] = in->A[i];
        }
        /* get nextchoice and nextbounds */
        count = (ctx->Cl == CL_ONE_OR_ZERO) ? CL_SIZE_MAX : 1;
        for (size_t i = 0; i < count; ++i) {
            nextchoice[i] = (*ctx->Cl)[i];
            nextbound[i] = ctx->B(ctx->E, En, ctx->F, Fn, in->n);
        }
        /* sort nextchoice and nextbound */
        if (ctx->Cl == CL_ONE_OR_ZERO && nextbound[1] < nextbound[0]) {
            const unsigned temp_nextbound = nextbound[0];
            const bool temp_nextchoice = nextchoice[0];
            nextbound[0] = nextbound[1];
            nextchoice[0] = nextchoice[1];
            nextbound[1] = temp_nextbound;
            nextchoice[1] = temp_nextchoice;
        }
    }

    for (size_t i = 0; i < count; ++i) {
        if (in->has_optimality_cuts && nextbound[i] >= ctx->opt_P) {
            ++ctx->optimality_cuts;
            return;
        }
        ctx->X[l] = nextchoice[i];
        _bb_solve(in, ctx, l + 1);
    }
}

/**
 * @brief Prints the encountered optimal solution
 *
 * @param in data parsed at input
 * @param ctx "global" references
 */
static void
_bb_solution_print(struct bb_input *in, struct _bb_ctx *ctx)
{
    ssize_t i, last_idx;

    if (ctx->opt_P == UINT_MAX) {
        puts("Inviável");
        return;
    }
    for (i = in->m - 1; i >= 0; --i) {
        if (ctx->opt_X[i] == true) {
            last_idx = i;
            break;
        }
    }
    for (i = 0; i < last_idx; ++i)
        if (ctx->opt_X[i] == true) printf("%zu ", i + 1);
    if (ctx->opt_X[i] == true) printf("%zu\n", i + 1);
    printf("%u\n", ctx->opt_P);
}

void
bb_solve(struct bb_input *in, bb_fn fn_bounding)
{
    struct _bb_ctx ctx = { .B = fn_bounding,
                           .opt_P = UINT_MAX,
                           .opt_X = calloc(in->m, sizeof *ctx.opt_X),
                           .E = calloc(in->m, sizeof *ctx.E),
                           .F = calloc(in->m, sizeof *ctx.F),
                           .X = calloc(in->m, sizeof *ctx.X),
                           .lens_S = calloc(in->l, sizeof *ctx.lens_S) };
    struct timeval t1, t2;
    double elapsed_time;

    if (!ctx.opt_X || !ctx.E || !ctx.F || !ctx.X || !ctx.lens_S) {
        perror("calloc()");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&t1, NULL);
    _bb_solve(in, &ctx, 0);
    gettimeofday(&t2, NULL);

    elapsed_time =
        (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;

    _bb_solution_print(in, &ctx);
    fprintf(stderr,
            "Visited nodes: %u\n"
            "Elapsed time: %.17G ms\n"
            "Optimality cuts: %u\n"
            "Feasibility cuts: %u\n",
            ctx.visited_nodes, elapsed_time, ctx.optimality_cuts,
            ctx.feasibility_cuts);

    free(ctx.opt_X);
    free(ctx.E);
    free(ctx.F);
    free(ctx.X);
    free(ctx.lens_S);
}
