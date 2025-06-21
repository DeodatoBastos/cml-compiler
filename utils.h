#ifndef UTIL_H
#define UTIL_H

ASTNode *new_stmt_node(StmtKind stmt_kind, const char *name);
ASTNode *new_expr_node(ExprKind expr_kind, const char *name);
void print_tree(ASTNode *node, int depth);
void free_ast(ASTNode *node);

void print_token(TokenType, const char*);
char *copy_str(char *s);

void print_help(const char *program_name);

#endif

