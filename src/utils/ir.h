#ifndef IR_H
#define IR_H

#include "bitset.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>

#define SAVED_REGISTERS 13

#define SP_REGISTER -2
#define A0_REGISTER -10
#define A1_REGISTER -11
#define A7_REGISTER -17
#define X0_REGISTER 0
#define FP_REGISTER -8
#define RA_REGISTER -1
#define T0_REGISTER -5

typedef enum SourceKind { CONST_SRC, REG_SRC, VAR_SRC } SourceKind;

typedef enum Instruction {
    MOV,
    LI,
    LUI,
    AUIPC,
    LOAD,
    STORE,

    ADD,
    SUB,
    MUL,
    DIV,
    REM,
    SLL,
    SRA,
    SRL,
    NOP,

    COMMENT,

    LABEL,

    JUMP,
    JUMP_REG,

    BEQ,
    BNE,
    BLE,
    BLT,
    BGE,
    BGT,

    CALL,
    ECALL
} Instruction;

typedef struct IR_Node {
    struct IR_Node *next, *prev;
    struct IR_Node *target;

    BucketList *var_src;
    Instruction instruction;
    SourceKind src_kind;

    int dest;
    int src1, src2;
    long imm;

    char *comment;
    int address;

    BitSet *live_in, *live_out;
} IRNode;

typedef struct IntermediateRepresentation {
    struct IR_Node *head;
    struct IR_Node *tail;

    int next_temp_reg;
    int next_while;
    int next_if;
    int last_address;
} IR;

IR *new_ir();
int register_new_temp(IR *ir);
int register_new_while(IR *ir);
int register_new_if(IR *ir);
void print_ir(IR *ir, FILE *out);
void free_ir(IR *ir);

IRNode *new_ir_node(Instruction instruction);
void ir_insert_node(IR *ir, IRNode *node);

// rd <- rs1
void ir_insert_mov(IR *ir, int dest, int src1);
// rd <- imm
void ir_insert_li(IR *ir, int dest, long imm);
// rd <- imm << 12
void ir_insert_lui(IR *ir, int dest, int imm);
// rd <- pc + imm << 12
void ir_insert_auipc(IR *ir, int dest, int imm);
// `rd <- mem[imm + rs1]`
void ir_insert_load(IR *ir, int dest, int imm, int src1);
// `mem[imm + rs1] <- rs2`
void ir_insert_store(IR *ir, int src2, int imm, int src1);

// rd <- rs1 + imm
void ir_insert_addi(IR *ir, int dest, int src1, int imm);
// rd <- rs1 + rs2
void ir_insert_add(IR *ir, int dest, int src1, int src2);
// rd <- rs1 - rs2
void ir_insert_sub(IR *ir, int dest, int src1, int src2);
// rd <- rs1 * rs2
void ir_insert_mul(IR *ir, int dest, int src1, int src2);
// rd <- rs1 / rs2
void ir_insert_div(IR *ir, int dest, int src1, int src2);
// rd <- rs1 % rs2
void ir_insert_rem(IR *ir, int dest, int src1, int src2);
// rd <- rs1 << imm
void ir_insert_slli(IR *ir, int dest, int src1, int imm);
// rd <- rs1 << rs2
void ir_insert_sll(IR *ir, int dest, int src1, int src2);
// rd <- rs1 >> imm (insert mst of rs1)
void ir_insert_srai(IR *ir, int dest, int src1, int imm);
// rd <- rs1 >> rs2 (insert mst of rs1)
void ir_insert_srl(IR *ir, int dest, int src1, int src2);
// rd <- rs1 >> imm (insert 0)
void ir_insert_srai(IR *ir, int dest, int src1, int imm);
// rd <- rs1 >> rs2 (insert 0)
void ir_insert_srl(IR *ir, int dest, int src1, int src2);
// nothing
void ir_insert_nop(IR *ir);

// comment
void ir_insert_comment(IR *ir, char *comment);

// label:
IRNode *ir_insert_label(IR *ir, char *label);

// ra <- pc + 4;
// pc <- pc + imm (imm == label)
IRNode *ir_insert_jump(IR *ir, char *label);
// ra <- pc + 4;
// pc <- rs1
void ir_insert_jump_reg(IR *ir, int src1);

// rs1 == rs2 => pc <- pc + imm (imm == label)
IRNode *ir_insert_beq(IR *ir, int src1, int src2, int imm);
// rs1 != rs2 => pc <- pc + imm (imm == label)
IRNode *ir_insert_bne(IR *ir, int src1, int src2, int imm);
// rs1 <= rs2 => pc <- pc + imm (imm == label)
IRNode *ir_insert_ble(IR *ir, int src1, int src2, int imm);
// rs1 < rs2 => pc <- pc + imm (imm == label)
IRNode *ir_insert_blt(IR *ir, int src1, int src2, int imm);
// rs1 >= rs2 => pc <- pc + imm (imm == label)
IRNode *ir_insert_bge(IR *ir, int src1, int src2, int imm);
// rs1 > rs2 => pc <- pc + imm (imm == label)
IRNode *ir_insert_bgt(IR *ir, int src1, int src2, int imm);

// call function: call label
void ir_insert_call(IR *ir, char *label);
// call function: ecall
void ir_insert_ecall(IR *ir);

#endif
