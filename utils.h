#ifndef UTIL_H
#define UTIL_H

#include "ast.h"

/* Function new_stmt_node creates a new statement
 * node for syntax tree construction
 */
ASTNode *new_stmt_node(StmtKind stmt_kind, const char *name);

/* Function new_exp_node creates a new expression 
 * node for syntax tree construction
 */
ASTNode *new_expr_node(ExprKind expr_kind, const char *name);

/* procedure get_return_node gets the return node associated
 * to a function to verify if it respects the type definition
 */
ASTNode *get_return_node(ASTNode* node);

/* procedure print_tree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void print_tree(ASTNode *node, int depth);


/* procedure free_ast frees all memory associated with the
 * ast
 */
void free_ast(ASTNode *node);

/* Procedure print_token prints a token 
 * and its lexeme to the listing file
 */
void print_token(TokenType, const char*);


/* Procedure type_str gets the string type
 * of the enum
 */
const char* type_str(ExprType type);

/* Procedure var_type_str gets the string variabletype
 * type of the enum
 */
const char* var_type_str(ExprKind kind);

void print_help(const char *program_name);

#endif

