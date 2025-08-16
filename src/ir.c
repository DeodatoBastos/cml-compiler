#include "ir.h"
#include <stdio.h>
#include <stdlib.h>

IR *new_ir() {
    IR *ir = (IR *)malloc(sizeof(IR));
    ir->head = NULL;
    ir->tail = NULL;
    ir->next_temp_reg = 1;
    ir->last_address = 0;
    return ir;
}

int register_new_temp(IR *ir) {
    int id = ir->next_temp_reg++;
    return id;
}

const char *instruction_to_string(Instruction instr) {
    switch (instr) {
    case NOP:
        return "NOP";
    case COMMENT:
        return "COMMENT";
    case PREPARE_STACK:
        return "PREPARE_STACK";

    case MOV:
        return "MOV";
    case LI:
        return "LI";
    case LUI:
        return "LUI";
    case AUIPC:
        return "AUIPC";
    case LOAD:
        return "LOAD";
    case STORE:
        return "STORE";

    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case DIV:
        return "DIV";
    case REM:
        return "REM";
    case SLL:
        return "SLL";
    case SRA:
        return "SRA";
    case SRL:
        return "SRL";

    case LABEL:
        return "LABEL";
    case JUMP:
        return "JUMP";
    case JUMP_REG:
        return "JUMP_REG";
    case RELATIVE_JUMP:
        return "REL_JUMP";

    case BEQ:
        return "BEQ";
    case BNE:
        return "BNEQ";
    case BLE:
        return "BLE";
    case BLT:
        return "BLT";
    case BGE:
        return "BGE";
    case BGQ:
        return "BGQ";
    case BGT:
        return "BGT";

    case CALL:
        return "CALL";
    case ECALL:
        return "ECALL";

    default:
        return "UNKNOWN";
    }
}

static void print_register(FILE *out, int reg) {
    if (reg == SP_REGISTER)
        fprintf(out, "sp");
    else if (reg == FP_REGISTER)
        fprintf(out, "fp");
    else if (reg == RA_REGISTER)
        fprintf(out, "ra");
    else if (reg == A0_REGISTER)
        fprintf(out, "a0");
    else if (reg == A1_REGISTER)
        fprintf(out, "a1");
    else if (reg == T0_REGISTER)
        fprintf(out, "t0");
    else if (reg == X0_REGISTER)
        fprintf(out, "x0");
    else
        fprintf(out, "t%d", reg);
}

void print_ir(IR *ir, FILE *out) {
    if (!ir)
        return;

    if (!out)
        out = stdout;

    IRNode *node = ir->head;
    while (node) {
        Instruction instr = node->instruction;

        if (instr == COMMENT && node->comment) {
            // node = node->next;
            // continue;
            fprintf(out, "# %s\n", node->comment);
        } else if (instr == LABEL) {
            if (node->src_kind == VAR_SRC)
                fprintf(out, "%s:\n", node->var_src->node->attr.name);
            else
                fprintf(out, "%s:\n", node->comment);
        } else {
            fprintf(out, "%s ", instruction_to_string(instr));
            switch (instr) {
            case MOV:
                print_register(out, node->dest);
                fprintf(out, ", ");
                print_register(out, node->src1);
                break;

            case LI:
            case LUI:
            case AUIPC:
                print_register(out, node->dest);
                fprintf(out, ", %d", node->imm);
                break;

            case LOAD:
                print_register(out, node->dest);
                fprintf(out, ", ");
                if (node->src_kind == VAR_SRC)
                    fprintf(out, "%s(", node->var_src->node->attr.name);
                else
                    fprintf(out, "%d(", node->imm);
                print_register(out, node->src1);
                fprintf(out, ")");
                break;

            case STORE:
                print_register(out, node->src2);
                fprintf(out, ", ");
                if (node->src_kind == VAR_SRC)
                    fprintf(out, "%s(", node->var_src->node->attr.name);
                else
                    fprintf(out, "%d(", node->imm);
                print_register(out, node->src1);
                fprintf(out, ")");
                break;

            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case REM:
            case SLL:
            case SRA:
            case SRL:
                print_register(out, node->dest);
                fprintf(out, ", ");
                print_register(out, node->src1);
                fprintf(out, ", ");
                if (node->src_kind == REG_SRC)
                    print_register(out, node->src2);
                else if (node->src_kind == CONST_SRC)
                    fprintf(out, "%d", node->imm);
                break;

            case BEQ:
            case BNE:
            case BLT:
            case BLE:
            case BGE:
            case BGQ:
            case BGT:
                print_register(out, node->src1);
                fprintf(out, ", ");
                print_register(out, node->src2);
                fprintf(out, ", ");
                fprintf(out, "%d", node->imm);
                break;

            case JUMP:
                if (node->src_kind == CONST_SRC)
                    fprintf(out, "%d", node->imm);
                else if (node->src_kind == VAR_SRC)
                    fprintf(out, "%s", node->var_src->node->attr.name);
                break;

            case JUMP_REG:
                print_register(out, node->src1);
                break;

            case RELATIVE_JUMP:
                fprintf(out, "%d", node->target->address);
                break;

            case CALL:
                fprintf(out, " ");
                if (node->src_kind == CONST_SRC)
                    fprintf(out, "%d", node->imm);
                else if (node->src_kind == VAR_SRC)
                    fprintf(out, "%s", node->comment);
                break;

            case ECALL:
            case PREPARE_STACK:
            case NOP:
            default:
                // No operands
                break;
            }
            fprintf(out, "\n");
        }
        node = node->next;
    }
}

void free_ir(IR *ir) {
    if (ir == NULL)
        return;

    IRNode *node = ir->head;
    IRNode *next = NULL;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }

    free(ir);
}

IRNode *new_ir_node(Instruction instruction) {
    IRNode *node = (IRNode *)malloc(sizeof(IRNode));
    node->next = NULL;
    node->target = NULL;
    node->var_src = NULL;
    node->instruction = instruction;
    node->src_kind = CONST_SRC;
    node->dest = X0_REGISTER;
    node->src1 = X0_REGISTER;
    node->src2 = X0_REGISTER;
    node->imm = 0;
    node->address = -1;
    return node;
}

void ir_insert_node(IR *ir, IRNode *node) {
    if (ir->head == NULL)
        ir->head = node;
    if (ir->tail == NULL)
        ir->tail = node;
    else {
        ir->tail->next = node;
        ir->tail = node;
    }
    if (node->instruction != COMMENT) {
        node->address = ir->last_address;
        ir->last_address += 4;
    }
}

void ir_insert_prepare_stack(IR *ir) {
    IRNode *node = new_ir_node(PREPARE_STACK);
    node->src_kind = CONST_SRC;

    ir_insert_node(ir, node);
}

void ir_insert_mov(IR *ir, int dest, int src1) {
    IRNode *node = new_ir_node(MOV);
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->dest = dest;

    ir_insert_node(ir, node);
}

void ir_insert_li(IR *ir, int dest, int imm) {
    IRNode *node = new_ir_node(LI);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_lui(IR *ir, int dest, int imm) {
    IRNode *node = new_ir_node(LUI);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_auipc(IR *ir, int dest, int imm) {
    IRNode *node = new_ir_node(AUIPC);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->imm = imm;

    ir_insert_node(ir, node);
}

IRNode *ir_insert_auipc_addr(IR *ir, int dest, BucketList *ref) {
    IRNode *node = new_ir_node(AUIPC);
    node->dest = dest;
    node->src_kind = VAR_SRC;
    node->var_src = ref;

    ir_insert_node(ir, node);
    return node;
}

void ir_insert_load(IR *ir, int dest, int imm, int src1) {
    IRNode *node = new_ir_node(LOAD);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_load_addr(IR *ir, int dest, BucketList *ref) {
    IRNode *node = new_ir_node(LOAD);
    node->dest = dest;
    node->src_kind = VAR_SRC;
    node->var_src = ref;

    ir_insert_node(ir, node);
}

void ir_insert_load_var_addr(IR *ir, int dest, int src1, BucketList *ref) {
    IRNode *node = new_ir_node(LOAD);
    node->src_kind = VAR_SRC;
    node->var_src = ref;
    node->dest = dest;
    node->src1 = src1;

    ir_insert_node(ir, node);
}

void ir_insert_store(IR *ir, int src2, int imm, int src1) {
    IRNode *node = new_ir_node(STORE);
    node->instruction = STORE;
    node->src_kind = REG_SRC;
    node->src2 = src2;
    node->src1 = src1;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_store_var_addr(IR *ir, int src2, int src1, BucketList *ref) {
    IRNode *node = new_ir_node(STORE);
    node->src_kind = VAR_SRC;
    node->var_src = ref;
    node->src2 = src2;
    node->src1 = src1;

    ir_insert_node(ir, node);
}

void ir_insert_addi(IR *ir, int dest, int src1, int imm) {
    IRNode *node = new_ir_node(ADD);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->src1 = src1;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_add(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(ADD);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_sub(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(SUB);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_mul(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(MUL);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_div(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(DIV);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_rem(IR *ir, int src1, int src2, int dest) {
    IRNode *node = new_ir_node(REM);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_slli(IR *ir, int dest, int src1, int imm) {
    IRNode *node = new_ir_node(SLL);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->src1 = src1;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_sll(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(SLL);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_srai(IR *ir, int dest, int src1, int imm) {
    IRNode *node = new_ir_node(SRA);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->src1 = src1;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_sra(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(SRA);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_srli(IR *ir, int dest, int src1, int imm) {
    IRNode *node = new_ir_node(SRL);
    node->dest = dest;
    node->src_kind = CONST_SRC;
    node->src1 = src1;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_srl(IR *ir, int dest, int src1, int src2) {
    IRNode *node = new_ir_node(SRL);
    node->dest = dest;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
}

void ir_insert_nop(IR *ir) {
    IRNode *node = new_ir_node(NOP);

    ir_insert_node(ir, node);
}

void ir_insert_comment(IR *ir, char *comment) {
    IRNode *node = new_ir_node(COMMENT);
    node->comment = comment;

    ir_insert_node(ir, node);
}

void ir_insert_label(IR *ir, char *ref) {
    IRNode *node = new_ir_node(LABEL);
    node->src_kind = CONST_SRC;
    node->comment = ref;

    ir_insert_node(ir, node);
}

void ir_insert_label_var(IR *ir, BucketList *ref) {
    IRNode *node = new_ir_node(LABEL);
    node->src_kind = VAR_SRC;
    node->var_src = ref;

    ir_insert_node(ir, node);
}

void ir_insert_jump(IR *ir, BucketList *ref) {
    IRNode *node = new_ir_node(JUMP);
    node->src_kind = VAR_SRC;
    node->var_src = ref;

    ir_insert_node(ir, node);
}

void ir_insert_rel_jump(IR *ir, IRNode *target) {
    IRNode *node = new_ir_node(RELATIVE_JUMP);
    node->target = target;

    ir_insert_node(ir, node);
}

void ir_insert_jump_reg(IR *ir, int src1) {
    IRNode *node = new_ir_node(JUMP_REG);
    node->src_kind = REG_SRC;
    node->src1 = src1;

    ir_insert_node(ir, node);
}

IRNode *ir_insert_jump_im(IR *ir, int imm) {
    IRNode *node = new_ir_node(JUMP);
    node->src_kind = CONST_SRC;
    node->imm = imm;

    ir_insert_node(ir, node);
    return node;
}

IRNode *ir_insert_beq(IR *ir, int src1, int src2, int imm) {
    IRNode *node = new_ir_node(BEQ);
    node->imm = imm;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
    return node;
}

IRNode *ir_insert_bne(IR *ir, int src1, int src2, int imm) {
    IRNode *node = new_ir_node(BNE);
    node->imm = imm;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
    return node;
}

IRNode *ir_insert_ble(IR *ir, int src1, int src2, int imm) {
    IRNode *node = new_ir_node(BLE);
    node->imm = imm;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
    return node;
}

IRNode *ir_insert_blt(IR *ir, int src1, int src2, int imm) {
    IRNode *node = new_ir_node(BLT);
    node->imm = imm;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
    return node;
}

IRNode *ir_insert_bge(IR *ir, int src1, int src2, int imm) {
    IRNode *node = new_ir_node(BGE);
    node->imm = imm;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
    return node;
}

IRNode *ir_insert_bgt(IR *ir, int src1, int src2, int imm) {
    IRNode *node = new_ir_node(BGT);
    node->imm = imm;
    node->src_kind = REG_SRC;
    node->src1 = src1;
    node->src2 = src2;

    ir_insert_node(ir, node);
    return node;
}

void ir_insert_calli(IR *ir, int imm) {
    IRNode *node = new_ir_node(CALL);
    node->src_kind = CONST_SRC;
    node->imm = imm;

    ir_insert_node(ir, node);
}

void ir_insert_call(IR *ir, char *label) {
    IRNode *node = new_ir_node(CALL);
    node->src_kind = VAR_SRC;
    node->comment = label;

    ir_insert_node(ir, node);
}

void ir_insert_ecall(IR *ir) {
    IRNode *node = new_ir_node(ECALL);
    node->src_kind = CONST_SRC;

    ir_insert_node(ir, node);
}
