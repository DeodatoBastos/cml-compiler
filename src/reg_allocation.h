#ifndef REG_ALLOCATION
#define REG_ALLOCATION

#include "ir.h"

/**
 * @brief Allocates physical registers using a graph-coloring algorithm.
 *
 * This function takes the intermediate representation, performs liveness analysis,
 * builds an interference graph, and then uses a graph-coloring algorithm to
 * assign physical registers to virtual temporary registers.
 *
 * @param ir A pointer to the Intermediate Representation to be processed.
 */
int *allocate_registers(IR *ir);

#endif // REG_ALLOC_H
