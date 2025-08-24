#ifndef OBJECTCODE_H
#define OBJECTCODE_H

#include "ir.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct ObjectCode {
    char assembly[64];
    bool include;

    struct ObjectCode *next;
} ObjectCode;

ObjectCode *new_obj_code();
ObjectCode *ir_to_obj_code(IR *ir, int *map, bool include_comments);
int asm_to_bin(char *stmt);
void free_obj_code(ObjectCode *obj_code);
void write_asm(ObjectCode *obj_code, FILE *f);

#endif
