#ifndef BB_H
#define BB_H

/** @brief Actor information */
struct bb_actor {
    /** acting cost */
    unsigned c;
    /** amount of groups the actor is part of */
    size_t s;
    /** group set this actor is part of */
    unsigned *sub_S;
};

/**
 * @brief Parsed input from stdin
 * @see bb_input_parse()
 */
struct bb_input {
    /** total amount of groups */
    size_t l;
    /** total amount of actors */
    size_t m;
    /** total amount of characters */
    size_t n;
    /** actors set */
    struct bb_actor *A;
    /** whether feasibility cuts are enabled */
    bool has_feasibility_cuts;
    /** whether optimality cuts are enabled */
    bool has_optimality_cuts;
};

/** @brief Helper-type for bounding function parameter */
typedef unsigned (*bb_fn)(
    struct bb_actor E[], size_t Em, struct bb_actor F[], size_t Fm, size_t n);

/**
 * @brief Parse and allocate resources from input
 *
 * @param in stores parsed input data
 * @return a boolean for success, either way a bb_input_cleanup() should be
 *      called
 */
_Bool bb_input_parse(struct bb_input *in);

/**
 * @brief Change input settings
 *
 * @param in input initialized with bb_input_parse()
 * @param feasibility_cuts whether feasibility cuts are enabled
 * @param optimality_cuts whether optimality cuts are enabled
 */
void bb_input_set(struct bb_input *in,
                  bool feasibility_cuts,
                  bool optimality_cuts);

/**
 * @brief Cleanup the resources allocated for @ref bb_input
 *
 * @param in parsed input data to be cleaned up
 */
void bb_input_cleanup(struct bb_input *in);

/**
 * @brief Solve the casting problem from `README.pdf` with Branch and Bound
 *      method
 *
 * @param in data parsed at input
 * @param fn_bounding bounding function
 */
void bb_solve(struct bb_input *in, bb_fn fn_bounding);

#endif /* BB_H */
