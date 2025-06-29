#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>
#include "ast.h"
#include "stack.h"

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec {
    int lineno;
    struct LineListRec * next;
} LineList;

/* The record in the bucket lists for
 * each variable, including name,
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec {
    LineList *lines;
    ASTNode *node;
    int scope;
    bool active;
    int memloc; /* memory location for variable */
    struct BucketListRec *next;
} BucketList;


/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(ASTNode *node, int scope, int loc);

/* Function st_activate activate the given
 * variable
 */
void st_activate(char *name, int scope);

/* Function st_lookup returns the ASTNode
 * of a variable or NULL if not found
 */
ASTNode *st_lookup(char *name, int scope);

/* Function st_lookup_soft returns the ASTNode
 * of a variable or NULL if not found it searchs
 * also for higher (closest to 0) scopes
 */
BucketList *st_lookup_soft(char *name, Stack *stack);

/* Function st_delete delete the last
 * entry with the given name
 */
void st_delete(char *name, int scope);

/* Function free_sym_tab delete all
 * entries in the symbol table
 */
void free_symtab();

/* Procedure printSymTab prints a formatted
 * list of the symbol table contents
 */
void print_symtab(FILE *listing);


void free_bucket_list(BucketList *l);
void free_line_list(LineList *l);

#endif
