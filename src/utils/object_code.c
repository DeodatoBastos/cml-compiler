#include "object_code.h"
#include "ir.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *get_reg(int *map, int reg_idx) {
    int temps[7] = {-5, -6, -7, -28, -29, -30, -31};
    char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "fp", "s1", "a0",
                           "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
                           "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

    if (reg_idx <= 0)
        return reg_names[-reg_idx];

    return reg_names[-temps[map[reg_idx]]];
}

ObjectCode *new_obj_code() {
    ObjectCode *obj = (ObjectCode *)malloc(sizeof(ObjectCode));
    obj->next = NULL;
    obj->include = true;
    strcpy(obj->assembly, "");

    return obj;
}

ObjectCode *ir_to_obj_code(IR *ir, int *map, bool include_comments) {
    ObjectCode *curr_obj = NULL, *obj_code = NULL;

    int i = 0;
    for (IRNode *node = ir->head; node != NULL; node = node->next) {
        if (obj_code == NULL) {
            obj_code = new_obj_code();
            curr_obj = obj_code;
        } else {
            curr_obj->next = new_obj_code();
            curr_obj = curr_obj->next;
        }

        switch (node->instruction) {
        case MOV:
            sprintf(curr_obj->assembly, "mv %s, %s", get_reg(map, node->dest),
                    get_reg(map, node->src1));
            break;
        case LI:
            sprintf(curr_obj->assembly, "li %s, 0x%lx", get_reg(map, node->dest), node->imm);
            break;
        case LUI:
            sprintf(curr_obj->assembly, "lui %s, %ld", get_reg(map, node->dest), node->imm);
            break;
        case AUIPC:
            sprintf(curr_obj->assembly, "auipc %s, %ld", get_reg(map, node->dest), node->imm);
            break;
        case LOAD:
            sprintf(curr_obj->assembly, "lw %s, %ld(%s)", get_reg(map, node->dest), node->imm,
                    get_reg(map, node->src1));
            break;
        case STORE:
            sprintf(curr_obj->assembly, "sw %s, %ld(%s)", get_reg(map, node->src2), node->imm,
                    get_reg(map, node->src1));
            break;

        case ADD:
            if (node->src_kind == CONST_SRC)
                sprintf(curr_obj->assembly, "addi %s, %s, %ld", get_reg(map, node->dest),
                        get_reg(map, node->src1), node->imm);
            else
                sprintf(curr_obj->assembly, "add %s, %s, %s", get_reg(map, node->dest),
                        get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case SUB:
            sprintf(curr_obj->assembly, "sub %s, %s, %s", get_reg(map, node->dest),
                    get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case MUL:
            sprintf(curr_obj->assembly, "mul %s, %s, %s", get_reg(map, node->dest),
                    get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case DIV:
            sprintf(curr_obj->assembly, "div %s, %s, %s", get_reg(map, node->dest),
                    get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case REM:
            sprintf(curr_obj->assembly, "rem %s, %s, %s", get_reg(map, node->dest),
                    get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case SLL:
            if (node->src_kind == CONST_SRC)
                sprintf(curr_obj->assembly, "slli %s, %s, %ld", get_reg(map, node->dest),
                        get_reg(map, node->src1), node->imm);
            else
                sprintf(curr_obj->assembly, "sll %s, %s, %s", get_reg(map, node->dest),
                        get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case SRL:
            if (node->src_kind == CONST_SRC)
                sprintf(curr_obj->assembly, "srli %s, %s, %ld", get_reg(map, node->dest),
                        get_reg(map, node->src1), node->imm);
            else
                sprintf(curr_obj->assembly, "srl %s, %s, %s", get_reg(map, node->dest),
                        get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case SRA:
            if (node->src_kind == CONST_SRC)
                sprintf(curr_obj->assembly, "srai %s, %s, %ld", get_reg(map, node->dest),
                        get_reg(map, node->src1), node->imm);
            else
                sprintf(curr_obj->assembly, "sra %s, %s, %s", get_reg(map, node->dest),
                        get_reg(map, node->src1), get_reg(map, node->src2));
            break;
        case NOP:
            sprintf(curr_obj->assembly, "nop");
            break;

        case COMMENT:
            sprintf(curr_obj->assembly, "# %s", node->comment);
            curr_obj->include = include_comments;
            break;

        case LABEL:
            sprintf(curr_obj->assembly, "\n%s:", node->comment);
            break;

        case JUMP_REG:
            sprintf(curr_obj->assembly, "jalr %s, %s, 0", get_reg(map, node->dest),
                    get_reg(map, RA_REGISTER));
            break;
        case JUMP:
            sprintf(curr_obj->assembly, "j %s", node->comment);
            break;

        case BEQ:
            sprintf(curr_obj->assembly, "beq %s, %s, %s", get_reg(map, node->src1),
                    get_reg(map, node->src2), node->comment);
            break;
        case BNE:
            sprintf(curr_obj->assembly, "bne %s, %s, %s", get_reg(map, node->src1),
                    get_reg(map, node->src2), node->comment);
            break;
        case BLE:
            // sprintf(currObj->assembly, "bge %s, %s, %d", get_reg(rm, node->src2),
            //         get_reg(rm, node->src1), node->imm);
            sprintf(curr_obj->assembly, "ble %s, %s, %s", get_reg(map, node->src1),
                    get_reg(map, node->src2), node->comment);
            break;
        case BLT:
            sprintf(curr_obj->assembly, "blt %s, %s, %s", get_reg(map, node->src1),
                    get_reg(map, node->src2), node->comment);
            break;
        case BGE:
            sprintf(curr_obj->assembly, "bge %s, %s, %s", get_reg(map, node->src1),
                    get_reg(map, node->src2), node->comment);
            break;
        case BGT:
            // sprintf(currObj->assembly, "blt %s, %s, %d", get_reg(rm, node->src2),
            //         get_reg(rm, node->src1), node->imm);
            sprintf(curr_obj->assembly, "bgt %s, %s, %s", get_reg(map, node->src1),
                    get_reg(map, node->src2), node->comment);
            break;

        case CALL:
            sprintf(curr_obj->assembly, "call %s", node->comment);
            break;
        case ECALL:
            sprintf(curr_obj->assembly, "ecall");
            break;
        }

        i++;
    }

    return obj_code;
}

void free_obj_code(ObjectCode *obj_code) {
    ObjectCode *node = obj_code;
    while (node != NULL) {
        obj_code = obj_code->next;
        free(node);
        node = obj_code;
    }
}

void write_asm(ObjectCode *obj_code, FILE *f) {
    ObjectCode *node = obj_code;
    while (node != NULL) {
        if (node->include && strlen(node->assembly) > 0)
            fprintf(f, "%s\n", node->assembly);
        node = node->next;
    }
}
