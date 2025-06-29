#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "stack.h"
#include "utils.h"
#include "global.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
static int location = 0;

/* counter for variables scopes */
static int scope = 0;

/* cache used to delete Parameters */
static int last_scope;

/* stack to store scopes */
Stack* stack;

static void type_error(ASTNode *n, char *message) {
    fprintf(listing, "\033[1;31mType Error\033 at line %d: %s\n", n->lineno, message);
    Error = true;
}

static void var_error(ASTNode *n, const char *var_type, char *msg, int scope) {
    fprintf(listing, "\033[1;31mVar Error\033: %s '%s' %s at line %d and scope %d\n", var_type, n->attr.name, msg, n->lineno, scope);
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
    if (n == NULL) return;
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
                    stack_push(stack, scope);
                    break;
                default:
                    break;
            }
            break;
        case Expr:
            switch (n->kind.expr) {
                BucketList *bucket;
                case VarDecl:
                case ArrDecl:
                    bucket = st_lookup_soft(n->attr.name);
                    if (bucket == NULL)
                        /* not yet in table, so treat as new definition */
                        st_insert(n, stack_top(stack), location++);
                    else if (bucket->node->kind.expr == FuncDecl)
                        /* function defined with the same name raise an error */
                        var_error(n, var_type_str(n->kind.expr), "has the name of a function already declared", stack_top(stack));
                    else if (bucket->scope != stack_top(stack))
                        /* not yet in table, so treat as new definition */
                        st_insert(n, stack_top(stack), location++);
                    else
                        /* already in table raise an error */
                        var_error(n, var_type_str(n->kind.expr), "redefined", stack_top(stack));
                    break;
                case FuncDecl:
                    if (st_lookup(n->attr.name, stack_top(stack)) == NULL)
                        /* not yet in table, so treat as new definition */
                        st_insert(n, stack_top(stack), location++);
                    else
                        /* already in table raise an error */
                        var_error(n, var_type_str(n->kind.expr), "redefined", stack_top(stack));
                    break;
                case ParamVar:
                case ParamArr:
                        /* not yet in table, so treat as new definition */
                        st_insert(n, scope+1, location++); // parameters are defined before entering a new scope
                    break;
                case Var:
                case Arr:
                case FuncCall:
                    bucket = st_lookup_soft(n->attr.name);
                    if (bucket == NULL)
                        /* not yet in table, so treat as new definition */
                        var_error(n, var_type_str(n->kind.expr), "never defined used", stack_top(stack));
                    else
                        /* already in table, so ignore location,
                         add line number of use only */
                        st_insert(n, bucket->scope, 0);
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
                st_delete(n->attr.name, stack_top(stack));
                break;
            case ParamVar:
            case ParamArr:
                st_delete(n->attr.name, last_scope);
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
                    last_scope = stack_top(stack);
                    stack_pop(stack);
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
    stack = stack_create();
    scope = 0;
    stack_push(stack, 0);
    traverse(syntaxTree, insert_node, delete_node);
    if (TraceAnalyze) {
        fprintf(listing, "\nSymbol table:\n\n");
        print_symtab(listing);
    }
    stack_destroy(stack);
    if (st_lookup("main", 0) == NULL) {
        fprintf(listing, "\033[1;Error\033: main function not found");
        Error = true;
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
                    stack_push(stack, scope);
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
                        st_activate(n->attr.name, stack_top(stack));
                        break;
                case ParamVar:
                case ParamArr:
                        st_activate(n->attr.name, scope+1); // how to deactive it?
                        break;
                default:
                    break;
            }
            break;
        default:
            break;
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
                BucketList *bucket;
                case VarDecl:
                case ArrDecl:
                    if (n->type != Integer)
                        type_error(n, "declaration of non-integer variable");
                    break;
                case Var:
                    bucket = st_lookup_soft(n->attr.name);
                    if (bucket == NULL) break;

                    node = bucket->node;
                    n->kind.expr = (node->kind.expr == ArrDecl) || (node->kind.expr == ParamArr) ? Arr : Var;
                    break;
                case FuncDecl:
                    if (n->type != Void) {
                        char *msg;
                        node = get_return_node(n->child[1]);
                        if (node == NULL) {
                            asprintf(&msg, "return stmt not found for the integer function '%s'", n->attr.name);
                            type_error(n, msg);
                            free(msg);
                        }
                        else if (n->type != node->type) {
                            asprintf(&msg, "return type of function '%s' must be integer", n->attr.name);
                            type_error(node, msg);
                        }
                    }
                    // pass for all the nodes and check if it is ParamArr or ParamVar
                    delete_decls(n->child[0]);
                    break;
                case FuncCall:
                    bucket = st_lookup_soft(n->attr.name);
                    if (bucket == NULL) break;

                    node = bucket->node;
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
                    last_scope = stack_top(stack);
                    stack_pop(stack);
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
    stack = stack_create();
    scope = 0;
    stack_push(stack, scope);
    traverse(syntaxTree, activate_node, check_node);
    stack_destroy(stack);
}
