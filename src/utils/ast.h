#ifndef AST_H
#define AST_H

#define MAXCHILDREN 3

typedef int TokenType;

typedef enum { Stmt, Expr } NodeKind;
typedef enum { Compound, If, While, Return, Read, Write, Assign } StmtKind;
typedef enum {
    Op,
    Const,
    VarDecl,
    Var,
    ParamVar,
    ArrDecl,
    Arr,
    ParamArr,
    FuncDecl,
    FuncCall
} ExprKind;
/* ExpType is used for type checking */
typedef enum { Void, Integer, Boolean } ExprType;

typedef struct ASTNode {
    int lineno;
    int scope;
    int temp_reg;

    NodeKind node_kind;
    ExprType type; /* for type checking of exps */
    union {
        StmtKind stmt;
        ExprKind expr;
    } kind;

    union {
        TokenType op;
        int val;
        char *name;
    } attr;

    struct ASTNode *child[MAXCHILDREN];
    struct ASTNode *sibling;
} ASTNode;

#endif
