#include "analyze.h"
#include "../global.h"
#include "../utils/ast.h"
#include "../utils/queue.h"
#include "../utils/stack.h"
#include "../utils/symtab.h"
#include "../utils/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* counter for global variable memory locations */
static unsigned int global_address = 0x10008000;

/* counters for stack frame offsets */
static int param_offset = 0;
static int local_offset = 0;

/* counter for variables scopes */
static int scope = 0;
/* cache used to delete Parameters */
static int last_scope;

/* stack to store scopes */
Stack *stack;

static void type_error(ASTNode *n, char *message) {
    fprintf(listing, "\033[1;31mType Error\033[0m at line %d: %s\n", n->lineno, message);
    Error = true;
}

static void var_error(ASTNode *n, const char *var_type, char *msg, int scope) {
    fprintf(listing, "\033[1;31mVar Error\033[0m: %s '%s' %s at line %d and scope %d\n", var_type,
            n->attr.name, msg, n->lineno, scope);
    Error = true;
}

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies pre_proc in preorder and post_proc
 * in postorder to tree pointed to by n
 */
static void traverse(ASTNode *n, void (*pre_proc)(ASTNode *), void (*post_proc)(ASTNode *)) {
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
static void null_proc(ASTNode *n) {
    if (n == NULL)
        return;
    else
        return;
}

/* Procedure insert_node inserts
 * identifiers stored in n into
 * the symbol table
 */
static void insert_node(ASTNode *n) {
    switch (n->node_kind) {
    case Stmt:
        switch (n->kind.stmt) {
        case Compound:
            scope++;
            s_push(stack, scope);
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
            n->scope = s_top(stack);
            int size = n->child[0] != NULL ? n->child[0]->attr.val : 1;

            if (bucket == NULL) { // not yet in table, so treat as new definition
                if (n->scope == 0) {
                    st_insert(n, n->scope, global_address, 0);
                    global_address += size * 4;
                } else {
                    local_offset -= 4 * size;
                    st_insert(n, n->scope, 0, local_offset);
                }
            } else if (bucket->node->kind.expr == FuncDecl) {
                /* function defined with the same name raise an error */
                var_error(n, var_type_str(n->kind.expr),
                          "has the name of a function already declared", s_top(stack));
            } else if (bucket->scope != s_top(stack)) { // new scope
                local_offset -= 4 * size;
                st_insert(n, n->scope, 0, local_offset);
            } else { // already in table raise an error
                var_error(n, var_type_str(n->kind.expr), "redefined", s_top(stack));
            }
            break;
        case FuncDecl:
            param_offset = 8;
            local_offset = 0;
            n->scope = s_top(stack);
            if (st_lookup(n->attr.name, s_top(stack)) == NULL) {
                st_insert(n, n->scope, 0, 0);
            } else { // already in table raise an error
                var_error(n, var_type_str(n->kind.expr), "redefined", s_top(stack));
            }
            break;
        case ParamVar:
        case ParamArr:
            // parameters are defined before entering a new scope
            n->scope = scope + 1;
            st_insert(n, scope + 1, 0, param_offset);
            param_offset += 4;
            break;
        case Var:
        case Arr:
        case FuncCall:
            bucket = st_lookup_soft(n->attr.name);
            if (bucket == NULL)
                /* not yet in table, so treat as new definition */
                var_error(n, var_type_str(n->kind.expr), "never defined used", s_top(stack));
            else if (n->kind.expr == FuncCall && bucket->node->kind.expr != FuncDecl)
                var_error(n, var_type_str(n->kind.expr), "called as function", s_top(stack));
            else {
                /* already in table, so ignore location,
                 add line number of use only */
                n->scope = bucket->scope;
                n->type = bucket->node->type;
                st_insert(n, bucket->scope, 0, 0);
                if ((bucket->node->kind.expr == ArrDecl) || (bucket->node->kind.expr == ParamArr))
                    n->kind.expr = Arr;
            }
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
    if (!n)
        return;
    delete_decls(n->sibling);

    if (n->node_kind == Expr) {
        switch (n->kind.expr) {
        case FuncDecl:
        case VarDecl:
        case ArrDecl:
            st_delete(n->attr.name, s_top(stack));
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
static void delete_node(ASTNode *n) {
    switch (n->node_kind) {
    case Stmt:
        switch (n->kind.stmt) {
        case Compound:
            // pass for all the nodes and check if it is FuncDecl or VarDecl or ArrDecl
            delete_decls(n->child[0]);
            last_scope = s_top(stack);
            s_pop(stack);
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
    stack = s_create();
    scope = 0;
    s_push(stack, 0);
    traverse(syntaxTree, insert_node, delete_node);
    // if (TraceAnalyze) {
    //     fprintf(listing, "\nSymbol table:\n\n");
    //     print_symtab(listing);
    // }
    s_destroy(stack);
    if (st_lookup("main", 0) == NULL) {
        fprintf(listing, "\033[1;31mError\033[0m: main function not found\n");
        Error = true;
    }
}

/* Procedure activate_node activates
 * identifiers stored in n into
 * the symbol table
 */
static void activate_node(ASTNode *n) {
    switch (n->node_kind) {
    case Stmt:
        switch (n->kind.stmt) {
        case Compound:
            scope++;
            s_push(stack, scope);
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
            st_activate(n->attr.name, s_top(stack));
            break;
        case ParamVar:
        case ParamArr:
            st_activate(n->attr.name, scope + 1); // how to deactive it?
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
    if (!n)
        return;
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
        case FuncDecl:
            if (n->type != Void) {
                char *msg;
                Queue *q = q_create();
                bool has_return = get_return_nodes(
                    n->child[1]->child[1],
                    q); // func_decl -> (params, compound) -> (local_decl, stmt_list)
                if (q_isEmpty(q)) {
                    asprintf(&msg, "return stmt not found for the integer function '%s'",
                             n->attr.name);
                    type_error(n, msg);
                    free(msg);
                }
                if (!has_return) {
                    asprintf(
                        &msg,
                        "return stmt not found in all control paths in the integer function '%s'",
                        n->attr.name);
                    type_error(n, msg);
                    free(msg);
                }
                while (!q_isEmpty(q)) {
                    node = q_front(q);
                    if (n->type != node->type) {
                        asprintf(&msg, "return type of function '%s' must be integer",
                                 n->attr.name);
                        type_error(node, msg);
                        free(msg);
                    }
                    q_pop(q);
                }
                q_destroy(q);
            }
            // pass for all the nodes and check if it is ParamArr or ParamVar
            delete_decls(n->child[0]);
            break;
        case FuncCall:
            bucket = st_lookup_soft(n->attr.name);
            // bucket = st_lookup(n->attr.name, 0);
            if (bucket == NULL)
                break;

            node = bucket->node;
            n->type = node->type;
            ASTNode *node_call = n->child[0];
            ASTNode *node_def = node->child[0];
            int num_call = 0, num_def = 0;
            char *msg;

            // check args types and quantity
            while (node_call != NULL && node_def != NULL) {
                if ((node_def->type != node_call->type) ||
                    (node_def->kind.expr == ParamArr &&
                     (node_call->kind.expr != Arr && node_call->kind.expr != ParamArr)) ||
                    (node_def->kind.expr == ParamVar &&
                     (node_call->kind.expr == Arr || node_call->kind.expr == ParamArr))) {
                    asprintf(&msg,
                             "argument '%s' of function '%s' must be '%s %s' instead of '%s %s'",
                             node_def->attr.name, node->attr.name, type_str(node_def->type),
                             var_type_str(node_def->kind.expr), type_str(node_call->type),
                             var_type_str(node_call->kind.expr));
                    type_error(node_call, msg);
                    free(msg);
                }
                node_call = node_call->sibling;
                node_def = node_def->sibling;
                num_call++;
                num_def++;
            }
            while (node_call != NULL) {
                node_call = node_call->sibling;
                num_call++;
            }
            while (node_def != NULL) {
                node_def = node_def->sibling;
                num_def++;
            }
            if (num_call != num_def) {
                asprintf(&msg, "too %s function '%s' expected '%d' arguments instead of '%d'",
                         num_def > num_call ? "few" : "much", n->attr.name, num_def, num_call);
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
            last_scope = s_top(stack);
            s_pop(stack);
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
void type_check(ASTNode *syntaxTree) {
    stack = s_create();
    scope = 0;
    s_push(stack, scope);
    traverse(syntaxTree, activate_node, check_node);
    if (TraceAnalyze) {
        fprintf(listing, "\nSymbol table:\n\n");
        print_symtab(listing);
    }
    s_destroy(stack);
}
