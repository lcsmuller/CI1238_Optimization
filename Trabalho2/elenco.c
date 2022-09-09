#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>

#include "bb.h"

/* alt bounding function provided at the README.pdf */
static unsigned
alt_bounding_fn(
    struct bb_actor E[], size_t Em, struct bb_actor F[], size_t Fm, size_t n)
{
    unsigned sum_cost = 0, min_cost = F[0].c;
    for (size_t i = 0; i < Em; ++i)
        sum_cost += E[i].c;
    for (size_t i = 1; i < Fm; ++i)
        if (F[i].c < min_cost) min_cost = F[i].c;
    return sum_cost + (n - Em) * min_cost;
}

/* default bounding function (should be slight better than alt_bounding_fn in
 *      most cases) */
static unsigned
default_bounding_fn(
    struct bb_actor E[], size_t Em, struct bb_actor F[], size_t Fm, size_t n)
{
    const unsigned leftover = n - Em;
    unsigned sum_cost = 0;
    unsigned memo[512]; // keep track of smaller costs
    size_t idx = 0;
    for (size_t i = 0; i < Em; ++i)
        sum_cost += E[i].c;
    // backtrack smaller costs
    memo[idx++] = F[0].c;
    for (size_t i = 1; i < Fm; ++i) {
        if (F[i].c <= memo[idx - 1]) {
            memo[idx++] = F[i].c;
        }
    }
    for (ssize_t i = idx - 1; i >= 0 && idx < leftover; --i)
        memo[idx++] = memo[i];
    for (size_t i = 0; i < leftover; ++i)
        sum_cost += memo[(idx - i) - 1];
    return sum_cost;
}

int
main(int argc, char *argv[])
{
    bb_fn fn = &default_bounding_fn; /**< bounding function */
    bool feasibility_cuts = true; /**< feasibility cuts */
    bool optimality_cuts = true; /**< optimality cuts */
    struct bb_input in = { 0 };

    for (int opt; (opt = getopt(argc, argv, "foah")) != -1;) {
        switch (opt) {
        case 'f':
            feasibility_cuts = false;
            break;
        case 'o':
            optimality_cuts = false;
            break;
        case 'a':
            fn = &alt_bounding_fn;
            break;
        case 'h':
        default:
            fprintf(stderr, "Usage ./%s [-f] [-o] [-a] [-h]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (!bb_input_parse(&in)) return EXIT_FAILURE;
    bb_input_set(&in, feasibility_cuts, optimality_cuts);

    bb_solve(&in, fn);

    bb_input_cleanup(&in);
    return EXIT_SUCCESS;
}
