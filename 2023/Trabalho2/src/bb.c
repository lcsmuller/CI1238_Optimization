#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <sys/time.h>

#include "bb.h"

/** @brief "Global" references structure */
struct _bb_ctx {
    /** bounding function */
    bb_fn B;
    /** current optimal amount of trips */
    unsigned opt_K;
    /** current optimal solution set */
    unsigned *opt_X;
    /** current feasible solution */
    unsigned *X;
    /** choices set (from set I) */
    unsigned *Cl;
    /** total of visited nodes */
    unsigned visited_nodes;
    /** total optimality cuts */
    unsigned optimality_cuts;
    /** total of feasibility cuts */
    unsigned feasibility_cuts;
    /**
     * @note helper buffer for _bb_Cl_compute()
     * accumulated weight for each (current) trip
     */
    unsigned *acc_weights;
};

/**
 * @brief Whether current item 'i' is restricted against item 'j'
 *
 * @param[in] in data parsed at input
 * @param[in] ctx "global" references
 * @param[in] i item i
 * @param[in] j item j
 * @return `true` if restricted items are in the same trip, `false` otherwise
 */
#define HAS_CONFLICT(in, ctx, i, j)                                           \
    ((ctx)->X[(i)] == (ctx)->X[(j)] && (in)->I[(i)].restrictions[(j)])

/**
 * @brief Check if restrictions for completed feasible solutions are met
 *
 * @param in data parsed at input
 * @param ctx "global" references
 * @param k current amount of trips
 * @return `true` if restrictions are validated, `false` otherwise
 */
static unsigned
_bb_check_restrictions(const struct bb_input *in,
                       const struct _bb_ctx *ctx,
                       const unsigned k)
{
    unsigned acc_weights[k]; ///< accumulated weights (per trip)

    memset(acc_weights, 0, k * sizeof *acc_weights);

    // get each trip accumulated weight and restrictions
    for (size_t i = 0; i < in->n; ++i) {
        acc_weights[ctx->X[i] - 1] += in->I[i].w;
        // check if restrictions for item 'i' are respected
        for (size_t j = 0; j < i; ++j)
            if (HAS_CONFLICT(in, ctx, i, j)) return false;
        for (size_t j = i + 1; j < in->n; ++j)
            if (HAS_CONFLICT(in, ctx, i, j)) return false;
    }

    for (size_t i = 0; i < k; ++i)
        if (acc_weights[i] > in->C) return false;
    return true;
}

/**
 * @brief Compute the Cl (choices) set for the current iteration
 *
 * @param in data parsed at input
 * @param ctx "global" references
 * @param l length of current feasible solution
 * @return Cl set's size
 */
static size_t
_bb_Cl_compute(const struct bb_input *in, struct _bb_ctx *ctx, const size_t l)
{
    size_t count = 0;

    if (l == in->n) return 0;

    if (in->has_feasibility_cuts && l != 0) {
        ++ctx->feasibility_cuts;
        memset(ctx->acc_weights, 0, in->n * sizeof *ctx->acc_weights);
        // get each trip accumulated weight
        for (size_t i = 0; i < l; ++i)
            ctx->acc_weights[ctx->X[i] - 1] += in->I[i].w;
        // pick trips where weight won't be surpassed
        for (size_t i = 0; i < l; ++i) {
            if (in->I[l].w + ctx->acc_weights[i] > in->C) continue;
            ctx->Cl[count++] = i + 1;
        }
        return count;
    }

    for (size_t i = 0; i < in->n; ++i)
        ctx->Cl[count++] = i + 1;
    return count;
}

/**
 * @brief Total amount of trips for currently picked items
 *
 * @param X current feasible solution
 * @param n current feasible solution size
 * @return total amount of trips
 */
static unsigned
_bb_get_trips_max(const unsigned X[], const size_t n)
{
    unsigned k = X[0];
    for (size_t i = 1; i < n; ++i)
        if (X[i] > k) k = X[i];
    return k;
}

struct _bb_next {
    unsigned choice;
    unsigned bound;
};

static int
_bb_next_cmp(const void *a, const void *b)
{
    return ((struct _bb_next *)a)->bound - ((struct _bb_next *)b)->bound;
}

/**
 * @brief Solve the transportation problem from `README.pdf` with the
 *      Branch and Bound method
 *
 * @param in data parsed at input
 * @param ctx "global" references
 * @param l index of current x node
 */
static void
_bb_solve(const struct bb_input *in, struct _bb_ctx *ctx, const size_t l)
{
    const unsigned k = _bb_get_trips_max(ctx->X, in->n);
    struct _bb_next next[in->n - 1];
    size_t count;

    ++ctx->visited_nodes;

    if (l == in->n && _bb_check_restrictions(in, ctx, k)) {
        if (k < ctx->opt_K) {
            ctx->opt_K = k;
            for (size_t i = 0; i < in->n; ++i)
                ctx->opt_X[i] = ctx->X[i];
        }
    }

    if ((count = _bb_Cl_compute(in, ctx, l)) != 0) {
        /* E = currently picked items; F = not yet picked items */
        const unsigned En = l + 1, Fn = in->n - En;
        const struct bb_item *E = in->I, *F = in->I + En;
        /* get nextchoice and nextbounds */
        for (size_t i = 0; i < count; ++i) {
            next[i].choice = ctx->Cl[i];
            next[i].bound = ctx->B(E, En, F, Fn, in->C, k);
        }
        /* sort choice from least to most trips */
        qsort(next, count, sizeof *next, &_bb_next_cmp);
    }

    for (size_t i = 0; i < count; ++i) {
        if (in->has_optimality_cuts && next[i].bound >= ctx->opt_K) {
            ++ctx->optimality_cuts;
            return;
        }
        ctx->X[l] = next[i].choice;
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
_bb_solution_print(const struct bb_input *in, const struct _bb_ctx *ctx)
{
    if (ctx->opt_K == UINT_MAX) {
        puts("Inviável");
        return;
    }
    for (size_t i = 0; i < in->n - 1; ++i)
        printf("%u ", ctx->opt_X[i]);
    printf("%u\n", ctx->opt_X[in->n - 1]);
    printf("%u\n", ctx->opt_K);
}

void
bb_solve(const struct bb_input *in, const bb_fn fn_bounding)
{
    struct _bb_ctx ctx = {
        .B = fn_bounding,
        .opt_K = UINT_MAX,
        .opt_X = calloc(in->n, sizeof *ctx.opt_X),
        .X = calloc(in->n, sizeof *ctx.X),
        .Cl = calloc(in->n, sizeof *ctx.Cl),
        .acc_weights = calloc(in->n, sizeof *ctx.acc_weights),
    };
    struct timeval t1, t2;
    double elapsed_time;

    if (!ctx.opt_X || !ctx.X || !ctx.Cl || !ctx.acc_weights) {
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
    free(ctx.X);
    free(ctx.Cl);
    free(ctx.acc_weights);
}
