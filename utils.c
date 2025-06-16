#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "utils.h"

ASTNode *new_node(NodeType type, ASTNode *left, ASTNode *right, const char *name) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = type;
    n->name = name ? strdup(name) : NULL;
    n->value = 0;
    n->left = left;
    n->right = right;
    return n;
}

ASTNode *new_id(const char *name) {
    return new_node(NODE_ID, NULL, NULL, name);
}

ASTNode *new_num(int val) {
    ASTNode *n = new_node(NODE_NUM, NULL, NULL, "Const");
    n->value = val;
    return n;
}

void print_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        fprintf(listing, "  ");
    }
}

const char* node_type_str(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_HEADER: return "HEADER";
        case NODE_DECL_LIST: return "DECL_LIST";
        case NODE_DECL: return "DECL";
        case NODE_VAR_DECL: return "VAR_DECL";
        case NODE_TYPE: return "TYPE";
        case NODE_FUNC_DECL: return "FUNC_DECL";
        case NODE_PARAM_LIST: return "PARAM_LIST";
        case NODE_PARAM: return "PARAM";
        case NODE_COMPOUND: return "COMPOUND";
        case NODE_LOCAL_DECL: return "LOCAL_DECL";
        case NODE_STMT_LIST: return "STMT_LIST";
        case NODE_EXPR_STMT: return "EXPR_STMT";
        case NODE_SEL_STMT: return "SEL_STMT";
        case NODE_ITER_STMT: return "ITER_STMT";
        case NODE_RETURN_STMT: return "RETURN_STMT";
        case NODE_ASSIGN: return "ASSIGN";
        case NODE_VAR: return "VAR";
        case NODE_SIMPLE_EXPR: return "SIMPLE_EXPR";
        case NODE_RELOP: return "RELOP";
        case NODE_ADD_EXPR: return "ADD_EXPR";
        case NODE_TERM: return "TERM";
        case NODE_CALL: return "CALL";
        case NODE_ARG_LIST: return "ARG_LIST";
        case NODE_WRITE: return "WRITE";
        case NODE_READ: return "READ";
        case NODE_NUM: return "NUM";
        case NODE_ID: return "ID";
        default: return "UNKNOWN";
    }
}

void print_tree(ASTNode *node, int depth) {
    if (!node) return;
    print_indent(depth);
    fprintf(listing, "%s", node_type_str(node->type));

    if (node->type == NODE_ID || node->type == NODE_TYPE || node->type == NODE_RELOP || node->type == NODE_VAR || node->type == NODE_CALL) {
        fprintf(listing, " (%s)", node->name);
    } else if (node->type == NODE_NUM) {
        fprintf(listing, " (%d)", node->value);
    }

    fprintf(listing, "\n");

    print_tree(node->left, depth + 1);
    print_tree(node->right, depth + 1);
}

void free_ast(ASTNode *node) {
    if (!node) return;

    free_ast(node->left);
    free_ast(node->right);

    if (node->name) free(node->name);

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

        case INCLUDE:
              fprintf(listing, "header: %s\n", tokenString);
              break;

        case ASSIGN: fprintf(listing, "=\n"); break;
        case LE: fprintf(listing, "<=\n"); break;
        case LT: fprintf(listing, "<\n"); break;
        case GT: fprintf(listing, ">\n"); break;
        case GE: fprintf(listing, ">=\n"); break;
        case EQ: fprintf(listing, "=\n"); break;
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
        case MOD: fprintf(listing, "/\n"); break;

        case ENDFILE: fprintf(listing, "EOF\n"); break;
        case NUM:
              fprintf(listing, "NUM, val = %s\n",tokenString);
              break;
        case ID:
              fprintf(listing, "ID, name = %s\n",tokenString);
              break;
        case ERROR:
              fprintf(listing, "ERROR: %s\n",tokenString);
              break;

        default:
            fprintf(listing, "Unknown token: %d\n",token);
    }
}

char * copy_str(char * s) {
    int n;
    char * t;
    if (s==NULL)
        return NULL;

    n = strlen(s)+1;
    t = malloc(n);

    if (t==NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else strcpy(t,s);

    return t;
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
