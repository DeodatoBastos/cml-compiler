#ifndef UTIL_H
#define UTIL_H

ASTNode *new_node(NodeType type, ASTNode *left, ASTNode *right, const char *name);
ASTNode *new_id(const char *name);
ASTNode *new_num(int val);
void print_tree(ASTNode *node, int depth);
void free_ast(ASTNode *node);

void print_token(TokenType, const char*);
char *copy_str(char *s);

void print_help(const char *program_name);

#endif

