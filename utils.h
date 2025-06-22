#ifndef UTIL_H
#define UTIL_H

#include "ast.h"

ASTNode *new_stmt_node(StmtKind stmt_kind, const char *name);
ASTNode *new_expr_node(ExprKind expr_kind, const char *name);
ASTNode *get_return_node(ASTNode* node);
void print_tree(ASTNode *node, int depth);
void free_ast(ASTNode *node);

void print_token(TokenType, const char*);
const char* type_str(ExprType type);
const char* var_type_str(ExprKind kind);
char *copy_str(char *s);

void print_help(const char *program_name);

#endif

