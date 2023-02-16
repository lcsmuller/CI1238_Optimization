#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bb.h"

#define BUF_SIZE 1024

#ifndef _BB_DEBUG
#define PRINT_DEBUG(in)
#else
static void
_bb_input_debug(const struct bb_input *in)
{
    fputs("Parsed input:", stderr);
    fprintf(stderr, "%zu %zu %u\n", in->n, in->p, in->C);
    for (size_t i = 0; i < in->n - 1; ++i)
        fprintf(stderr, "%u ", in->I[i].w);
    fprintf(stderr, "%u\n", in->I[in->n - 1].w);

    fputs("Restrictions:\n", stderr);
    for (size_t i = 0; i < in->n; ++i) {
        fprintf(stderr, "%zu: ", i);
        for (size_t j = 0; j < in->n - 1; ++j)
            fprintf(stderr, "%u ", in->I[i].restrictions[j]);
        fprintf(stderr, "%u\n", in->I[i].restrictions[in->n - 1]);
    }
}
#define PRINT_DEBUG(in) _bb_input_debug(in)
#endif

bool
bb_input_parse(struct bb_input *in)
{
    char buf[BUF_SIZE], *ptr = buf;
    size_t n, p;
    unsigned C;

    if (!fgets(buf, sizeof(buf), stdin)) {
        perror("fgets()");
        return false;
    }
    if (sscanf(buf, "%zu %zu %u", &n, &p, &C) != 3) {
        perror("sscanf()");
        return false;
    }
    *in = (struct bb_input){
        .n = n,
        .p = p,
        .C = C,
        .I = calloc(n, sizeof *in->I),
    };
    if (!in->I) {
        perror("calloc()");
        return false;
    }

    /* fill I-set */
    if (!fgets(buf, sizeof(buf), stdin)) {
        perror("fgets()");
        return false;
    }
    for (size_t i = 0; i < n; ++i) {
        in->I[i].w = (unsigned)strtoul(ptr, &ptr, 10);
        in->I[i].restrictions = calloc(n, sizeof *in->I[i].restrictions);
    }

    /* fill P-set (restrictions) */
    for (size_t i = 0; i < p; ++i) {
        unsigned a, b;

        if (!fgets(buf, sizeof(buf), stdin)) {
            perror("fgets()");
            return false;
        }
        if (sscanf(buf, "%u %u", &a, &b) != 2) {
            perror("sscanf()");
            return false;
        }
        in->I[a - 1].restrictions[b - 1] = in->I[b - 1].restrictions[a - 1] =
            true;
    }

    PRINT_DEBUG(in);
    return true;
}

void
bb_input_set(struct bb_input *in, bool feasibility_cuts, bool optimality_cuts)
{
    in->has_feasibility_cuts = feasibility_cuts;
    in->has_optimality_cuts = optimality_cuts;
}

void
bb_input_cleanup(struct bb_input *in)
{
    if (!in->I) return;
    for (size_t i = 0; i < in->n; ++i)
        free(in->I[i].restrictions);
    free(in->I);
}
