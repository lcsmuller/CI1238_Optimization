#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#include "bb.h"

/* alt bounding function provided at the README.pdf */
static unsigned
alt_bounding_fn(const struct bb_item E[],
                const size_t En,
                const struct bb_item F[],
                const size_t Fn,
                const unsigned C,
                const unsigned k)
{
    (void)F;
    (void)Fn;
    float estimated_k = E[0].w;
    for (size_t i = 1; i < En; ++i)
        estimated_k += E[i].w;
    estimated_k = ceil(estimated_k / C);
    return (k > estimated_k) ? k : estimated_k;
}

/* default bounding function (should be slight better than alt_bounding_fn in
 *      most cases) */
static unsigned
bounding_fn(const struct bb_item E[],
            const size_t En,
            const struct bb_item F[],
            const size_t Fn,
            const unsigned C,
            const unsigned k)
{
    float estimated_Ek = E[0].w, estimated_Fk = F[0].w;

    for (size_t i = 1; i < En; ++i)
        estimated_Ek += E[i].w;
    estimated_Ek = ceil(estimated_Ek / C);
    if (k > estimated_Ek) estimated_Ek = k;

    for (size_t i = 1; i < Fn; ++i)
        estimated_Fk += F[i].w;
    estimated_Fk = ceil(estimated_Fk / C);

    return estimated_Ek + estimated_Fk;
}

int
main(int argc, char *argv[])
{
    bool feasibility_cuts = true, optimality_cuts = true;
    struct bb_input in = { 0 };
    bb_fn fn = &bounding_fn;

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
