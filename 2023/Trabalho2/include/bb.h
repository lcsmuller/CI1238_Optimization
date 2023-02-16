#ifndef BB_H
#define BB_H

/** @brief Item information */
struct bb_item {
    /** item weight (in kg) */
    unsigned w;
    /** item restrictions (indexes) */
    bool *restrictions;
};

/** @brief trip's item pairs restriction */
struct bb_pair {
    /** item a and item b can't be carried together in a trip */
    struct bb_item *a, *b;
};

/**
 * @brief Parsed input from stdin
 * @see bb_input_parse()
 */
struct bb_input {
    /** total amount of items */
    size_t n;
    /** amount of restrictions */
    size_t p;
    /** maximum weight capacity */
    unsigned C;
    /** items set */
    struct bb_item *I;
    /** whether feasibility cuts are enabled */
    bool has_feasibility_cuts;
    /** whether optimality cuts are enabled */
    bool has_optimality_cuts;
};

/** @brief Helper-type for bounding function parameter */
typedef unsigned (*bb_fn)(const struct bb_item E[],
                          const size_t Em,
                          const struct bb_item F[],
                          const size_t Fm,
                          const unsigned C,
                          const unsigned k);

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
 * @brief Solve the transportation problem from `README.pdf` with the
 *      Branch and Bound method
 *
 * @param in data parsed at input
 * @param fn_bounding bounding function
 */
void bb_solve(const struct bb_input *in, const bb_fn fn_bounding);

#endif /* BB_H */
