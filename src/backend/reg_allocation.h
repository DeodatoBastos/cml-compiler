#ifndef REG_ALLOCATION
#define REG_ALLOCATION

#include "../utils/ir.h"

/**
 * @brief Number of available physical registers for allocation
 *
 * This constant defines how many physical registers are available for
 * the register allocator to use. If more virtual registers are needed
 * than this number allows, register spilling will be required.
 */
#define K 4

/**
 * @brief Allocate physical registers to virtual registers in IR
 *
 * Performs complete register allocation on the given intermediate representation
 * using graph coloring algorithm. The function:
 *
 * 1. Analyzes register liveness throughout the program
 * 2. Builds an interference graph showing which registers cannot share
 *    the same physical register due to overlapping lifetimes
 * 3. Colors the interference graph to assign physical register numbers
 * 4. Returns a mapping from virtual register IDs to physical register colors
 *
 * The returned mapping array is indexed by virtual register ID (1 to n)
 * and contains the assigned physical register color (0 to K-1).
 *
 * @param ir Pointer to IR structure containing virtual register usage
 * @return Array mapping virtual register IDs to physical register colors,
 *         or NULL on allocation failure. The caller is responsible for
 *         freeing this array when done.
 *
 * @note If register spilling is required (more than K registers needed
 *       simultaneously), the function will report a fatal error and set
 *       the global Error flag.
 *
 * @warning The function modifies the IR by adding liveness analysis
 *          information (live_in and live_out bitsets) to each instruction node.
 */
int *allocate_registers(IR *ir);

#endif // REG_ALLOCATION_H
