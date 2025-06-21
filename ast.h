#ifndef AST_H
#define AST_H

#define MAXCHILDREN 3

typedef int TokenType;

typedef enum {
    NODE_PROGRAM, NODE_HEADER, NODE_DECL_LIST, NODE_DECL, NODE_VAR_DECL, NODE_ARR_DECL,
    NODE_TYPE, NODE_FUNC_DECL, NODE_PARAM_LIST, NODE_PARAM, NODE_PARAM_ARR,
    NODE_COMPOUND,
    NODE_LOCAL_DECL, NODE_STMT_LIST, NODE_EXPR_STMT, NODE_SEL_STMT, NODE_ITER_STMT,
    NODE_RETURN_STMT, NODE_ASSIGN, NODE_VAR, NODE_ARR, NODE_SIMPLE_EXPR, NODE_RELOP,
    NODE_ADD_EXPR, NODE_TERM, NODE_CALL, NODE_ARG_LIST, NODE_WRITE, NODE_READ,
    NODE_NUM, NODE_ID,
} NodeType;

typedef enum {StmtK,ExpK} NodeKind;
typedef enum {IfK,RepeatK,AssignK,ReadK,WriteK} StmtKind;

typedef enum {OpK,ConstK,IdK} ExpKind;
typedef enum {Void,Integer,Boolean} ExpType;

typedef struct ASTNode {
    NodeType node_type;
    int lineno;

    // NodeKind node_kind;
    ExpType type;

    // union {
    //     StmtKind stmt;
    //     ExpKind exp;
    // } kind;

    // char* name;
    // int value;
    // char* op;
    union {
        TokenType op;
        int val;
        char *name;
    } attr;

    struct ASTNode *child[MAXCHILDREN];
    struct ASTNode *sibling;
} ASTNode;

#endif
