#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "global.h"
#include "utils.h"

ASTNode *new_node(NodeType node_type, const char *name) {
    ASTNode *n = (ASTNode *) malloc(sizeof(ASTNode));
    if (n == NULL) 
        fprintf(listing,"Out of memory error at line %d\n",lineno);

    n->node_type = node_type;
    n->attr.name = name ? strdup(name) : NULL;
    for (int i = 0; i < MAXCHILDREN; i++)
        n->child[i] = NULL;
    n->sibling = NULL;
    n->lineno = lineno;
    return n;
}

ASTNode *new_num(int val) {
    ASTNode *n = new_node(NODE_NUM, NULL);
    n->attr.val = val;
    return n;
}

void print_indent(int depth) {
    for (int i = 0; i < depth; i++) {
        fprintf(listing, "  ");
    }
}

const char* node_type_str(NodeType node_type) {
    switch (node_type) {
        case NODE_VAR_DECL: return "VAR_DECL";
        case NODE_ARR_DECL: return "ARR_DECL";
        case NODE_TYPE: return "TYPE";
        case NODE_FUNC_DECL: return "FUNC_DECL";
        case NODE_PARAM: return "PARAM";
        case NODE_PARAM_ARR: return "PARAM_ARR";
        case NODE_COMPOUND: return "COMPOUND";
        case NODE_SEL_STMT: return "SEL_STMT";
        case NODE_ITER_STMT: return "ITER_STMT";
        case NODE_RETURN_STMT: return "RETURN_STMT";
        case NODE_READ: return "READ";
        case NODE_WRITE: return "WRITE";
        case NODE_ASSIGN: return "ASSIGN";
        case NODE_VAR: return "VAR";
        case NODE_ARR: return "ARR";
        case NODE_RELOP: return "RELOP";
        case NODE_ADDOP: return "ADDOP";
        case NODE_MULOP: return "MULOP";
        case NODE_CALL: return "CALL";
        case NODE_NUM: return "NUM";
        default: return "UNKNOWN";
    }
}

void print_tree(ASTNode *node, int depth) {
    while(node != NULL) {
        print_indent(depth);
        fprintf(listing, "%s", node_type_str(node->node_type));

        if (node->node_type == NODE_VAR_DECL || node->node_type == NODE_ARR_DECL ||
            node->node_type == NODE_FUNC_DECL || node->node_type == NODE_PARAM ||
            node->node_type == NODE_PARAM_ARR || node->node_type == NODE_SEL_STMT ||
            node->node_type == NODE_ARR || node->node_type == NODE_VAR ||
            node->node_type == NODE_TYPE || node->node_type == NODE_CALL) {
            fprintf(listing, " (%s)\n", node->attr.name);
        } else if (node->node_type == NODE_NUM) {
            fprintf(listing, " (%d)\n", node->attr.val);
        } else if (node->node_type == NODE_RELOP ||
                   node->node_type == NODE_ADDOP ||
                   node->node_type == NODE_MULOP) {
            fprintf(listing, ": ");
            print_token(node->attr.op,"\0");
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

    if (node->attr.name != NULL &&
        node->node_type != NODE_RELOP &&
        node->node_type != NODE_ADDOP &&
        node->node_type != NODE_MULOP &&
        node->node_type != NODE_NUM)
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

char * copy_str(char *s) {
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
