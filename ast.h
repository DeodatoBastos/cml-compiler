#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM, NODE_HEADER, NODE_DECL_LIST, NODE_DECL, NODE_VAR_DECL, NODE_TYPE,
    NODE_FUNC_DECL, NODE_PARAM_LIST, NODE_PARAM, NODE_COMPOUND, NODE_LOCAL_DECL,
    NODE_STMT_LIST, NODE_EXPR_STMT, NODE_SEL_STMT, NODE_ITER_STMT, NODE_RETURN_STMT,
    NODE_ASSIGN, NODE_VAR, NODE_SIMPLE_EXPR, NODE_RELOP, NODE_ADD_EXPR, NODE_TERM,
    NODE_CALL, NODE_ARG_LIST, NODE_WRITE, NODE_READ,
    NODE_NUM, NODE_ID,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* name;
    int value;
    char* op;

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *sibling;
} ASTNode;

#endif
