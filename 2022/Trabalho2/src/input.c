#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bb.h"

#define BUF_SIZE 1024

bool
bb_input_parse(struct bb_input *in)
{
    char buf[BUF_SIZE];
    size_t l, m, n;

    if (!fgets(buf, sizeof(buf), stdin)) {
        perror("fgets()");
        return false;
    }
    if (sscanf(buf, "%zu %zu %zu", &l, &m, &n) != 3) {
        perror("sscanf()");
        return false;
    }
    *in = (struct bb_input){
        .l = l,
        .m = m,
        .n = n,
        .A = calloc(m, sizeof *in->A),
    };
    if (!in->A) {
        perror("calloc()");
        return false;
    }

    for (size_t i = 0; i < m; ++i) {
        unsigned c;
        size_t s;

        if (!fgets(buf, sizeof(buf), stdin)) {
            perror("fgets()");
            return false;
        }
        if (sscanf(buf, "%u %zu", &c, &s) != 2) {
            perror("sscanf()");
            return false;
        }
        in->A[i] = (struct bb_actor){
            .c = c,
            .s = s,
            .sub_S = calloc(s, sizeof *in->A[i].sub_S),
        };
        if (!in->A[i].sub_S) {
            perror("calloc()");
            return false;
        }
        for (size_t j = 0; j < in->A[i].s; ++j) {
            if (!fgets(buf, sizeof(buf), stdin)) {
                perror("fgets()");
                return false;
            }
            in->A[i].sub_S[j] = (unsigned)strtoul(buf, NULL, 10);
        }
    }
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
    if (in->A) {
        for (size_t i = 0; i < in->m; ++i)
            if (in->A[i].sub_S) free(in->A[i].sub_S);
        free(in->A);
    }
}
