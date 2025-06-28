#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "utils.h"
#include "global.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
static int location = 0;
static int scope = 0;

static void type_error(ASTNode *n, char *message) {
    fprintf(listing, "Type error at line %d: %s\n", n->lineno, message);
    Error = true;
}

static void var_error(ASTNode *n, const char *var_type, char *msg, int scope) {
    fprintf(listing, "Var error: %s '%s' %s at line %d and scope %d\n", var_type, n->attr.name, msg, n->lineno, scope);
    Error = true;
}

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies pre_proc in preorder and post_proc
 * in postorder to tree pointed to by n
 */
static void traverse(ASTNode *n,
               void (* pre_proc) (ASTNode *),
               void (* post_proc) (ASTNode *)) {
    if (n != NULL) {
        pre_proc(n);
        {
            for (int i = 0; i < MAXCHILDREN; i++)
                traverse(n->child[i], pre_proc, post_proc);
        }
        post_proc(n);
        traverse(n->sibling, pre_proc, post_proc);
    }
}

/* null_proc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void null_proc(ASTNode * n) {
    if (n==NULL) return;
    else return;
}

/* Procedure insert_node inserts
 * identifiers stored in n into
 * the symbol table
 */
static void insert_node(ASTNode * n) {
    switch (n->node_kind) {
        case Stmt:
            switch (n->kind.stmt) {
                case Compound:
                    scope++;
                    break;
                default:
                    break;
            }
            break;
        case Expr:
            switch (n->kind.expr) {
                case VarDecl:
                case ArrDecl:
                case FuncDecl:
                    if (st_lookup(n->attr.name, scope) == NULL)
                        /* not yet in table, so treat as new definition */
                        st_insert(n, scope, location++);
                    else
                        /* already in table raise an error */
                        var_error(n, var_type_str(n->kind.expr), "redefined", scope);
                    break;
                case ParamVar:
                case ParamArr:
                        // /* not yet in table, so treat as new definition */
                        st_insert(n, scope+1, location++);
                    break;
                case Var:
                case Arr:
                case FuncCall:
                    if (st_lookup_soft(n->attr.name, scope) == NULL)
                        /* not yet in table, so treat as new definition */
                        var_error(n, var_type_str(n->kind.expr), "never defined used", scope);
                    else
                        /* already in table, so ignore location,
                         add line number of use only */
                        st_insert(n, scope, 0);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* Procedure activate_node activates 
 * identifiers stored in n into
 * the symbol table
 */
static void activate_node(ASTNode * n) {
    switch (n->node_kind) {
        case Stmt:
            switch (n->kind.stmt) {
                case Compound:
                    scope++;
                    break;
                default:
                    break;
            }
            break;
        case Expr:
            switch (n->kind.expr) {
                case VarDecl:
                case ArrDecl:
                case FuncDecl:
                        st_activate(n->attr.name, scope);
                        break;
                case ParamVar:
                case ParamArr:
                        st_activate(n->attr.name, scope+1);
                        break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void delete_decls(ASTNode *n) {
    if (!n) return;

    delete_decls(n->sibling);

    if (n->node_kind == Expr) {
        switch (n->kind.expr) {
            case FuncDecl:
            case VarDecl:
            case ArrDecl:
                st_delete(n->attr.name, scope);
                break;
            case ParamVar:
            case ParamArr:
                st_delete(n->attr.name, scope+1);
                break;
            default:
                break;
        }
    }

}


/* Procedure delete_node deletes
 * identifiers stored in n into
 * the symbol table
 */
static void delete_node(ASTNode * n) {
    switch (n->node_kind) {
        case Stmt:
            switch (n->kind.stmt) {
                case Compound:
                    // pass for all the nodes and check if it is FuncDecl or VarDecl or ArrDecl
                    delete_decls(n->child[0]);
                    scope--;
                    break;
                default:
                    break;
            }
            break;
        case Expr:
            switch (n->kind.expr) {
                case FuncDecl:
                    // pass for all the nodes and check if it is ParamArr or ParamVar
                    delete_decls(n->child[0]);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* Function build_symtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void build_symtab(ASTNode *syntaxTree) {
    traverse(syntaxTree, insert_node, delete_node);
    if (TraceAnalyze) {
        fprintf(listing, "\nSymbol table:\n\n");
        print_symtab(listing);
    }
}

/* Procedure check_node performs
 * type checking at a single tree node
 */
static void check_node(ASTNode *n) {
    if (!n) return;
    switch (n->node_kind) {
        case Expr:
            switch (n->kind.expr) {
                ASTNode *node;
                case Var:
                    node = st_lookup_soft(n->attr.name, scope);
                    n->kind.expr = node->kind.expr == ArrDecl ? Arr : Var;
                    break;
                case FuncDecl:
                    if (n->type != Void) {
                        node = get_return_node(n->child[1]);
                        if (node == NULL)
                            type_error(n, "return stmt not found");
                        else if (n->type != node->type)
                            type_error(node, "return type must be the same type as the function definition");
                    }
                    break;
                case FuncCall:
                    node = st_lookup_soft(n->attr.name, scope);
                    n->type = node->type;
                    ASTNode *tc = n->child[0];
                    ASTNode *td = node->child[0];
                    int nc = 0, nd = 0;
                    char *msg;

                    // check args types and quantity
                    while (tc != NULL && td != NULL) {
                        if ((tc->type != td->type) ||
                            (td->kind.expr == ParamArr && (tc->kind.expr != Arr && tc->kind.expr != ParamArr)) ||
                            (td->kind.expr == ParamVar && (tc->kind.expr == Arr || tc->kind.expr == ParamArr))) {
                            asprintf(&msg, "argument '%s' of function '%s' must be '%s %s' instead of '%s %s'",
                                    td->attr.name, node->attr.name,
                                    type_str(td->type), var_type_str(td->kind.expr),
                                    type_str(tc->type), var_type_str(tc->kind.expr));
                            type_error(tc, msg);
                            free(msg);
                        }
                        tc = tc->sibling;
                        td = td->sibling;
                        nc++;
                        nd++;
                    }
                    while (tc != NULL) {
                        tc = tc->sibling;
                        nc++;
                    }
                    while (td != NULL) {
                        td = td->sibling;
                        nd++;
                    }
                    if (nc != nd) {
                        asprintf(&msg, "too %s function '%s' expected '%d' arguments instead of '%d'",
                                nd > nc ? "few" : "much", n->attr.name, nd, nc);
                        type_error(n, msg);
                        free(msg);
                    }

                    // pass for all the nodes and check if it is ParamArr or ParamVar
                    delete_decls(n->child[0]);
                    break;
                default:
                    break;
            }
            break;
        case Stmt:
            switch (n->kind.stmt) {
                case If:
                    if (n->child[0]->type != Boolean)
                        type_error(n->child[0], "if test is not Boolean");
                    break;
                case While:
                    if (n->child[0]->type != Boolean)
                        type_error(n->child[0], "while test is not Boolean");
                    break;
                case Assign:
                    if (n->child[1]->type != Integer)
                        type_error(n->child[1], "assignment of non-integer value");
                    break;
                case Write:
                    if (n->child[0]->type != Integer)
                        type_error(n->child[0], "write of non-integer value");
                    break;
                case Compound:
                    // pass for all the nodes and check if it is FuncDecl or VarDecl or ArrDecl
                    delete_decls(n->child[0]);
                    scope--;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* Procedure type_check performs type checking 
 * by a postorder syntax tree traversal
 */
void type_check(ASTNode * syntaxTree) {
    traverse(syntaxTree, activate_node, check_node);
}
