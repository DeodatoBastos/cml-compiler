#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "global.h"
#include "utils.h"
#include "symtab.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash(char * key) {
    int temp = 0;
    int i = 0;
    while (key[i] != '\0') {
        temp = ((temp << SHIFT) + key[i]) % SIZE;
        ++i;
    }
    return temp;
}

/* the hash table */
static BucketList *hashTable[SIZE];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(ASTNode *node, int scope, int loc) {
    int h = hash(node->attr.name);
    BucketList *l =  hashTable[h];
    while ((l != NULL) && (strcmp(node->attr.name, l->node->attr.name) != 0) && l->scope == scope) l = l->next;

    if (l == NULL) { /* variable not yet in table */
        l = (BucketList *) malloc(sizeof(BucketList));

        l->node = node;
        l->scope = scope;
        l->active = true;
        l->memloc = loc;

        l->lines = (LineList *) malloc(sizeof(LineList));
        l->lines->lineno = node->lineno;
        l->lines->next = NULL;


        l->next = hashTable[h];
        hashTable[h] = l;
    } else { /* found in table, so just add line number */
        LineList *t = l->lines;
        while (t->next != NULL) t = t->next;
        t->next = (LineList *) malloc(sizeof(LineList));
        t->next->lineno = node->lineno;
        t->next->next = NULL;
    }
}


/* Function st_lookup returns the ASTNode
 * of a variable or NULL if not found
 */
ASTNode *st_lookup(char *name, int scope) {
    int h = hash(name);
    BucketList *l =  hashTable[h];
    while ((l != NULL) && (strcmp(name, l->node->attr.name) != 0) && scope != l->scope) l = l->next;

    // if (l == NULL || !l->active) return NULL;
    if (l == NULL) return NULL;
    else return l->node;
}

/* Function st_delete delete the last
 * entry with the given name
 */
void st_delete(char *name, int scope) {
    int h = hash(name);
    BucketList *l = hashTable[h];
    while ((l != NULL) && (strcmp(name, l->node->attr.name) != 0) && scope != l->scope) l = l->next;

    if (l == NULL) {
        fprintf(listing, "Entry '%s' not found in scope '%d'\n", name, scope);
        return;
    }
    // fprintf(listing, "Deleting '%s' in scope '%d'\n", name, scope);

    l->active = false;
}

void free_symtab() {
    for(int i = 0; i < SIZE; i++) {
        free_bucket_list(hashTable[i]);
        hashTable[i] = NULL;
    }
}

/* Procedure printSymTab prints a formatted 
 * list of the symbol table contents 
 */
void print_symtab(FILE *listing) {
    fprintf(listing, "Variable Name  Type  Var Type  Scope  Location  Active   Line Numbers\n");
    fprintf(listing, "-------------  ----  --------  -----  --------  ------   ------------\n");

    for (int i=0; i < SIZE; i++) {
        if (hashTable[i] != NULL) {
            BucketList *l = hashTable[i];

            while (l != NULL) {
                LineList *t = l->lines;
                fprintf(listing, "%-13s  ", l->node->attr.name);
                fprintf(listing, "%-4s  ", type_str(l->node->type));
                fprintf(listing, "%-8s  ", var_type_str(l->node->kind.expr));
                fprintf(listing, "%-5d  ", l->scope);
                fprintf(listing, "%-8d  ", l->memloc);
                fprintf(listing, "%-6s   ", l->active ? "true" : "false");

                while (t != NULL) {
                    fprintf(listing, "%4d", t->lineno);
                    t = t->next;
                }

                fprintf(listing, "\n");
                l = l->next;
            }
        }
    }
} /* printSymTab */

void free_bucket_list(BucketList *l) {
    BucketList *temp = l->next;
    while (temp->next != NULL) {
        free_line_list(l->lines);
        free(l);
        l = temp;
        temp = temp->next;
    }
    free(l);
}

void free_line_list(LineList *l) {
    LineList *temp = l->next;
    while (temp->next != NULL) {
        free(l);
        l = temp;
        temp = temp->next;
    }
    free(l);
}
