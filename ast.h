#ifndef AST_H
#define AST_H

#define MAXCHILDREN 3

typedef int TokenType;

typedef enum {
    NODE_VAR_DECL, NODE_ARR_DECL, NODE_TYPE, NODE_FUNC_DECL, NODE_PARAM,
    NODE_PARAM_ARR, NODE_COMPOUND, NODE_SEL_STMT, NODE_ITER_STMT,
    NODE_RETURN_STMT, NODE_READ, NODE_WRITE, NODE_ASSIGN, NODE_VAR,
    NODE_ARR, NODE_RELOP, NODE_ADDOP, NODE_MULOP,
    NODE_CALL, NODE_NUM,
} NodeType;

typedef enum {Stmt, Exp} NodeKind;
typedef enum {
    Compound, If, While, Return,
    Read, Write, Assign
} StmtKind;
typedef enum {
    Op, Const,
    VarDecl, Var, ParamVar,
    ArrDecl, Arr, ParamArr,
    FuncDecl, FuncCall
} ExpKind;
typedef enum {Void, Integer, Boolean} ExpType;

typedef struct ASTNode {
    NodeType node_type;
    int lineno;

    // NodeKind node_kind;
    ExpType type;

    // union {
    //     StmtKind stmt;
    //     ExpKind exp;
    // } kind;

    union {
        TokenType op;
        int val;
        char *name;
    } attr;

    struct ASTNode *child[MAXCHILDREN];
    struct ASTNode *sibling;
} ASTNode;

#endif
