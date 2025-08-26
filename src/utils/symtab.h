#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"
#include "stack.h"
#include <stdio.h>

/* ST_SIZE is the size of the hash table */
#define ST_SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the list of line numbers of the source
 * code in which a variable is referenced
 */
typedef struct LineListRec {
    int lineno;
    struct LineListRec *next;
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
    int offset;           /* stack offset */
    unsigned int address; /* memory location for global variable */
    struct BucketListRec *next;
} BucketList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * addr = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(ASTNode *node, int scope, unsigned int addr, int offset);

/* Function st_activate activate the given
 * variable
 */
void st_activate(char *name, int scope);

/* Function st_lookup returns the ASTNode
 * of a variable or NULL if not found
 */
BucketList *st_lookup(char *name, int scope);

/* Function st_lookup_soft returns the ASTNode
 * of a variable or NULL if not found it searchs
 * also for higher (closest to 0) scopes
 */
BucketList *st_lookup_soft(char *name);

/* Function st_delete delete the last
 * entry with the given name
 */
void st_delete(char *name, int scope);

/* Function free_symtab delete all
 * entries in the symbol table
 */
void free_symtab();

/* Procedure print symtab prints a formatted
 * list of the symbol table contents
 */
void print_symtab(FILE *listing);

/* Function free_bucket_list deletes
 * an BucketList element and all its
 * LineList elements
 */
void free_bucket_list(BucketList *l);

/* Function free_line_list deletes
 * an LineList element
 * */
void free_line_list(LineList *l);

#endif
