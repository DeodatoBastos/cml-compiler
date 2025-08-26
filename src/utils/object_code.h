#ifndef OBJECTCODE_H
#define OBJECTCODE_H

#include "ir.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * @struct ObjectCode
 * @brief Represents a single assembly instruction in a linked list
 *
 * Each ObjectCode node contains a RISC-V assembly instruction string,
 * a flag indicating whether to include it in output, and a pointer
 * to the next instruction in the sequence.
 */
typedef struct ObjectCode {
    char assembly[64];
    bool include;

    struct ObjectCode *next;
} ObjectCode;

/**
 * @brief Create a new ObjectCode node
 *
 * Allocates memory for a new ObjectCode structure and initializes it
 * with default values (empty assembly string, include=true, next=NULL).
 *
 * @return Pointer to newly allocated ObjectCode node, or NULL on failure
 */
ObjectCode *new_obj_code();

/**
 * @brief Convert intermediate representation to object code
 *
 * Translates an IR linked list into a corresponding ObjectCode linked list
 * containing RISC-V assembly instructions. Uses a register mapping table
 * to convert virtual registers to physical RISC-V registers.
 *
 * @param ir Pointer to IR structure containing instruction list to convert
 * @param map Array mapping virtual register indices to physical register indices
 * @param include_comments Whether to include comment instructions in output
 * @return Pointer to head of ObjectCode linked list, or NULL on failure
 */
ObjectCode *ir_to_obj_code(IR *ir, int *map, bool include_comments);

/**
 * @brief Convert assembly instruction to binary (declaration only)
 *
 * Converts a RISC-V assembly instruction string to its binary representation.
 * Note: Implementation not provided in the source file shown.
 *
 * @param stmt RISC-V assembly instruction string to convert
 * @return Binary representation of the instruction
 */
int asm_to_bin(char *stmt);

/**
 * @brief Free ObjectCode linked list
 *
 * Deallocates memory for an entire ObjectCode linked list, traversing
 * from the head node and freeing each node in sequence.
 *
 * @param obj_code Pointer to head of ObjectCode linked list to free
 */
void free_obj_code(ObjectCode *obj_code);

/**
 * @brief Write assembly code to file
 *
 * Writes all assembly instructions from an ObjectCode linked list to
 * the specified file. Only includes comments if the 'include'
 * flag is true and the assembly string is non-empty.
 *
 * @param obj_code Pointer to head of ObjectCode linked list to write
 * @param f File pointer to write assembly instructions to
 */
void write_asm(ObjectCode *obj_code, FILE *f);

/**
 * @brief Write binary code to file
 *
 * Converts all assembly instructions from an ObjectCode linked list to
 * binary format and writes them to the specified file. Uses asm_to_bin()
 * to convert each assembly instruction to its binary representation.
 * Only processes comments if the 'include' flag is true.
 *
 * @param obj_code Pointer to head of ObjectCode linked list to convert and write
 * @param f File pointer to write binary instructions to
 */
void write_bin(ObjectCode *obj_code, FILE *f);

#endif
