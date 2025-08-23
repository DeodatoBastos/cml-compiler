#ifndef CGEN_H
#define CGEN_H

#include "../utils/ast.h"
#include "../utils/ir.h"

/**
 * @brief Generates the Intermediate Representation (IR) from an Abstract Syntax Tree (AST).
 *
 * This is the main entry point for the code generation phase. It traverses the
 * provided AST and produces a linear sequence of IR instructions, which is
 * the precursor to the final assembly code.
 *
 * @param tree The root of the completed and type-checked Abstract Syntax Tree.
 * @return A pointer to the generated Intermediate Representation structure.
 */
IR *gen_ir(ASTNode *node);

#endif
