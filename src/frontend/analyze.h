#ifndef ANALYZE_H
#define ANALYZE_H

#include "../utils/ast.h"

/* Function build_symtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void build_symtab(ASTNode *node);

/* Procedure type_check performs type checking 
 * by a postorder syntax tree traversal
 */
void type_check(ASTNode *node);

#endif
