#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "global.h"
#include "utils.h"

ASTNode *new_stmt_node(StmtKind kind, const char *name) {
    ASTNode * n = (ASTNode *) malloc(sizeof(ASTNode));
    if (n == NULL) {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        return NULL;
    }

    n->node_kind = Stmt;
    n->kind.stmt = kind;
    n->lineno = lineno;
    n->attr.name = name ? strdup(name) : NULL;

    n->sibling = NULL;
    for (int i = 0; i < MAXCHILDREN; i++)
        n->child[i] = NULL;

    return n;
}

ASTNode *new_expr_node(ExprKind kind, const char *name) {
    ASTNode * n = (ASTNode *) malloc(sizeof(ASTNode));
    if (n == NULL) {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        return NULL;
    }

    n->node_kind = Expr;
    n->kind.expr = kind;
    n->lineno = lineno;
    n->attr.name = name ? strdup(name) : NULL;

    n->sibling = NULL;
    for (int i = 0; i < MAXCHILDREN; i++)
        n->child[i] = NULL;

    return n;
}

ASTNode *get_return_node(ASTNode* node) {
    ASTNode *r = NULL;
    if (node == NULL) return r;

    for (int i = 0; i < MAXCHILDREN; i++) {
        if (r == NULL)
            r = get_return_node(node->child[i]);
    }

    if (r == NULL)
        r = get_return_node(node->sibling);

    if (node->node_kind == Stmt && node->kind.stmt == Return)
        r = node;

    return r;
}

void print_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        fprintf(listing, "  ");
    }
}

const char* node_kind_str(ASTNode* node) {
    if (node->node_kind == Stmt) {
        switch (node->kind.stmt) {
            case Compound: return "Compound";
            case If: return "If";
            case While: return "While";
            case Return: return "Return";
            case Read: return "Read: ";
            case Write: return "Write";
            case Assign: return "Assign to:";
            default: return "Unknown StmtNode kind";
        }
    } else if (node->node_kind == Expr) {
        switch (node->kind.expr) {
            case Op: return "Op: ";
            case Const: return "Const: ";
            case VarDecl: return "Var declaration: ";
            case ArrDecl: return "Array declaration: ";
            case Var: return "Var: ";
            case Arr: return "Array: ";
            case ParamVar: return "Parameter Var: ";
            case ParamArr: return "Parameter Array: ";
            case FuncDecl: return "Function declaration: ";
            case FuncCall: return "Function call: ";
            default: return "Unknown ExprNode kind";
        }
    } else return "Unkown node kind";
}

const char* type_str(ExprType type) {
    switch (type) {
        case Void: return "void";
        case Integer: return "int";
        case Boolean: return "bool";
        default: return "unkown type";
    }
}

const char* var_type_str(ExprKind kind) {
    switch (kind) {
        case VarDecl:
        case Var:
            return "Var";
        case ParamVar:
            return "P Var";
        case ArrDecl:
        case Arr:
            return "Arr";
        case ParamArr:
            return "P Arr";
        case FuncDecl:
        case FuncCall:
            return "Func";
        default: return "unkown type";
    }
}

void print_tree(ASTNode *node, int depth) {
    while(node != NULL) {
        print_indent(depth);
        fprintf(listing, "%s", node_kind_str(node));

        if(node->node_kind == Stmt) {
            if(node->kind.stmt == Read) {
                fprintf(listing, "%s\n", node->attr.name);
            } else {
                fprintf(listing, "\n");
            }
        } else if (node->node_kind == Expr) {
            if(node->kind.expr == Const) {
                fprintf(listing, "(%d)\n", node->attr.val);
            }
            else if (node->kind.expr == Op) {
                print_token(node->attr.op,"\0");
            } else if(node->kind.expr == FuncDecl || node->kind.expr == FuncCall) {
                fprintf(listing, "%s (%s)\n", node->attr.name, type_str(node->type));
            } else if(node->kind.expr == VarDecl || node->kind.expr == Var || node->kind.expr == ParamVar ||
                      node->kind.expr == ArrDecl || node->kind.expr == Arr || node->kind.expr == ParamArr) {
                fprintf(listing, "%s\n", node->attr.name);
            } else {
                fprintf(listing, "\n");
            }
        } else {
            fprintf(listing, "\n");
        }

        for (int i = 0; i < MAXCHILDREN; i++)
            print_tree(node->child[i], depth + 1);

        node = node->sibling;
    }
}

void free_ast(ASTNode *node) {
    if (!node) return;

    if (node->attr.name != NULL && !(
            node->node_kind == Expr && (
                node->kind.expr == Op ||
                node->kind.expr == Const
            )
        ))
        free(node->attr.name);

    for (int i = 0; i < MAXCHILDREN; i++) {
        free_ast(node->child[i]);
    }

    free_ast(node->sibling);

    free(node);
}

void print_token(TokenType token, const char* tokenString) {
    switch (token) {
        case WRITE:
        case READ:
        case INT:
        case VOID:
        case RETURN:
        case WHILE:
        case IF:
        case ELSE:
              fprintf(listing, "reserved word: %s\n", tokenString);
              break;

        case ASSIGN: fprintf(listing, "=\n"); break;
        case LE: fprintf(listing, "<=\n"); break;
        case LT: fprintf(listing, "<\n"); break;
        case GT: fprintf(listing, ">\n"); break;
        case GE: fprintf(listing, ">=\n"); break;
        case EQ: fprintf(listing, "==\n"); break;
        case NE: fprintf(listing, "!=\n"); break;

        case LPAREN: fprintf(listing, "(\n"); break;
        case RPAREN: fprintf(listing, ")\n"); break;
        case LBRACE: fprintf(listing, "{\n"); break;
        case RBRACE: fprintf(listing, "}\n"); break;
        case LBRACK: fprintf(listing, "[\n"); break;
        case RBRACK: fprintf(listing, "]\n"); break;

        case SEMICOLON: fprintf(listing, ";\n"); break;
        case COMMA: fprintf(listing, ",\n"); break;

        case ADD: fprintf(listing, "+\n"); break;
        case SUB: fprintf(listing, "-\n"); break;
        case MUL: fprintf(listing, "*\n"); break;
        case DIV: fprintf(listing, "/\n"); break;
        case MOD: fprintf(listing, "%%\n"); break;

        case ENDFILE: fprintf(listing, "EOF\n"); break;
        case NUM:
              fprintf(listing, "NUM, val = %s\n", tokenString);
              break;
        case ID:
              fprintf(listing, "ID, name = %s\n", tokenString);
              break;
        case ERROR:
              fprintf(listing, "ERROR: %s\n", tokenString);
              break;

        default:
            fprintf(listing, "Unknown token: %d\n", token);
    }
}

void print_help(const char *program_name) {
    printf("Usage: %s [file]\n", program_name);
    printf("Options:\n");
    printf("  --ts      Enable tracing of the scanner (lexer)\n");
    printf("  --tp      Enable tracing of the parser\n");
    printf("  --ta      Enable tracing of the analyzer\n");
    printf("  --tc      Enable tracing of the code generation\n");
    printf("  --help    Show this help message\n");
}
