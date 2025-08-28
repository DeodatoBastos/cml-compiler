#ifndef IR_H
#define IR_H

#include "bitset.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>

#define SAVED_REGISTERS 13

/** @name RISC-V Register Definitions
 * @brief Predefined register indices for common RISC-V registers
 * @{
 */
#define SP_REGISTER -2  /**< Stack pointer register */
#define A0_REGISTER -10 /**< Argument/return value register 0 */
#define A1_REGISTER -11 /**< Argument/return value register 1 */
#define A7_REGISTER -17 /**< Argument register 7 (syscall number) */
#define X0_REGISTER 0   /**< Zero register (always contains 0) */
#define FP_REGISTER -8  /**< Frame pointer register */
#define RA_REGISTER -1  /**< Return address register */
#define T0_REGISTER -5  /**< Temporary register 0 */
/** @} */

/**
 * @enum SourceKind
 * @brief Specifies the type of source operand in an instruction
 */
typedef enum SourceKind { CONST_SRC, REG_SRC } SourceKind;

/**
 * @enum Instruction
 * @brief RISC-V instruction types supported by the IR
 */
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

/**
 * @struct IR_Node
 * @brief Single instruction node in the IR linked list
 *
 * Represents one instruction with all its operands, addressing information,
 * and links to adjacent instructions. Also includes liveness analysis data
 * for register allocation optimization.
 */
typedef struct IRNode {
    struct IRNode *next, *prev;
    struct IRNode *target;

    Instruction instruction;
    SourceKind src_kind;

    int dest;
    int src1, src2;
    long imm;

    char *comment;
    int address;

    BitSet *live_in, *live_out;
} IRNode;

/**
 * @struct IntermediateRepresentation
 * @brief Main IR structure containing instruction list and metadata
 *
 * Manages the doubly-linked list of IR instructions along with counters
 * for generating unique temporary registers, labels, and tracking addresses.
 */
typedef struct IntermediateRepresentation {
    struct IRNode *head;
    struct IRNode *tail;

    int next_temp_reg;
    int next_while;
    int next_if;
    int last_address;
} IR;

/**
 * @brief Create a new IR structure
 *
 * Allocates and initializes a new intermediate representation with
 * empty instruction list and reset counters.
 *
 * @return Pointer to newly allocated IR structure, or NULL on failure
 */
IR *new_ir();

/**
 * @brief Register a new temporary register
 *
 * Generates a unique temporary register ID and increments the counter.
 *
 * @param ir Pointer to IR structure
 * @return Unique temporary register ID
 */
int register_new_temp(IR *ir);

/**
 * @brief Register a new while loop label
 *
 * Generates a unique while loop label ID and increments the counter.
 *
 * @param ir Pointer to IR structure
 * @return Unique while loop label ID
 */
int register_new_while(IR *ir);

/**
 * @brief Register a new if statement label
 *
 * Generates a unique if statement label ID and increments the counter.
 *
 * @param ir Pointer to IR structure
 * @return Unique if statement label ID
 */
int register_new_if(IR *ir);

/**
 * @brief Print IR instructions to file
 *
 * Outputs the entire IR instruction sequence in human-readable format
 * to the specified file stream (or stdout if NULL).
 *
 * @param ir Pointer to IR structure to print
 * @param out File stream to write to (stdout if NULL)
 */
void print_ir(IR *ir, FILE *out);

/**
 * @brief Free IR structure and all nodes
 *
 * Deallocates the entire IR structure including all instruction nodes
 * and their associated memory (comments, bitsets).
 *
 * @param ir Pointer to IR structure to free
 */
void free_ir(IR *ir);

/**
 * @brief Create a new IR instruction node
 *
 * Allocates and initializes a new instruction node with the specified
 * instruction type and default values for all other fields.
 *
 * @param instruction Type of instruction for this node
 * @return Pointer to newly allocated IRNode, or NULL on failure
 */
IRNode *new_ir_node(Instruction instruction);

/**
 * @brief Insert node into IR instruction list
 *
 * Adds the specified node to the end of the IR instruction list
 * and assigns it an address (except for COMMENT instructions).
 *
 * @param ir Pointer to IR structure
 * @param node Pointer to node to insert
 */
void ir_insert_node(IR *ir, IRNode *node);

/**
 * @brief Insert node into IR instruction list
 *
 * Adds the specified node to the end of the IR instruction list
 * and assigns it an address (except for COMMENT instructions).
 *
 * @param ir Pointer to IR structure
 * @param node Pointer to node to insert
 */
void ir_insert_node(IR *ir, IRNode *node);

/** @name Data Movement Instructions
 * @brief Functions for inserting data movement instructions
 * @{
 */

/**
 * @brief Insert move instruction: rd ← rs1
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 */
void ir_insert_mov(IR *ir, int dest, int src1);

/**
 * @brief Insert load immediate: rd ← imm
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param imm Immediate value
 */
void ir_insert_li(IR *ir, int dest, long imm);

/**
 * @brief Insert load upper immediate: rd ← imm << 12
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param imm Immediate value
 */
void ir_insert_lui(IR *ir, int dest, int imm);

/**
 * @brief Insert add upper immediate to PC: rd ← pc + imm << 12
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param imm Immediate value
 */
void ir_insert_auipc(IR *ir, int dest, int imm);

/**
 * @brief Insert load word: rd ← mem[imm + rs1]
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param imm Immediate offset
 * @param src1 Base address register
 */
void ir_insert_load(IR *ir, int dest, int imm, int src1);

/**
 * @brief Insert store word: mem[imm + rs1] ← rs2
 * @param ir Pointer to IR structure
 * @param src2 Source data register
 * @param imm Immediate offset
 * @param src1 Base address register
 */
void ir_insert_store(IR *ir, int src2, int imm, int src1);

/** @} */

/** @name Arithmetic Instructions
 * @brief Functions for inserting arithmetic and logical instructions
 * @{
 */

/**
 * @brief Insert add immediate: rd ← rs1 + imm
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param imm Immediate value
 */
void ir_insert_addi(IR *ir, int dest, int src1, int imm);

/**
 * @brief Insert add: rd ← rs1 + rs2
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void ir_insert_add(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert subtract: rd ← rs1 - rs2
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void ir_insert_sub(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert multiply: rd ← rs1 * rs2
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 First source register
 * @param src2 Second source register
 */
void ir_insert_mul(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert divide: rd ← rs1 / rs2
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Dividend register
 * @param src2 Divisor register
 */
void ir_insert_div(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert remainder: rd ← rs1 % rs2
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Dividend register
 * @param src2 Divisor register
 */
void ir_insert_rem(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert shift left logical immediate: rd ← rs1 << imm
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param imm Shift amount
 */
void ir_insert_slli(IR *ir, int dest, int src1, int imm);

/**
 * @brief Insert shift left logical: rd ← rs1 << rs2
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param src2 Shift amount register
 */
void ir_insert_sll(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert shift right arithmetic immediate: rd ← rs1 >> imm (sign-extend)
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param imm Shift amount
 */
void ir_insert_srai(IR *ir, int dest, int src1, int imm);

/**
 * @brief Insert shift right arithmetic: rd ← rs1 >> rs2 (sign-extend)
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param src2 Shift amount register
 */
void ir_insert_sra(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert shift right logical immediate: rd ← rs1 >> imm (zero-extend)
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param imm Shift amount
 */
void ir_insert_srli(IR *ir, int dest, int src1, int imm);

/**
 * @brief Insert shift right logical: rd ← rs1 >> rs2 (zero-extend)
 * @param ir Pointer to IR structure
 * @param dest Destination register
 * @param src1 Source register
 * @param src2 Shift amount register
 */
void ir_insert_srl(IR *ir, int dest, int src1, int src2);

/**
 * @brief Insert no operation
 * @param ir Pointer to IR structure
 */
void ir_insert_nop(IR *ir);

/** @} */

/** @name Comments and Labels
 * @brief Functions for inserting comments and labels
 * @{
 */

/**
 * @brief Insert comment line
 * @param ir Pointer to IR structure
 * @param comment Comment text
 */
void ir_insert_comment(IR *ir, char *comment);

/**
 * @brief Insert label definition
 * @param ir Pointer to IR structure
 * @param label Label name
 * @return Pointer to created label node
 */
IRNode *ir_insert_label(IR *ir, char *label);

/** @} */

/** @name Control Flow Instructions
 * @brief Functions for inserting jumps and branches
 * @{
 */

/**
 * @brief Insert unconditional jump: ra ← pc + 4; pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param label Target label name
 * @return Pointer to created jump node
 */
IRNode *ir_insert_jump(IR *ir, char *label);

/**
 * @brief Insert jump to register: ra ← pc + 4; pc ← rs1
 * @param ir Pointer to IR structure
 * @param src1 Register containing target address
 */
void ir_insert_jump_reg(IR *ir, int src1);

/**
 * @brief Insert branch if equal: if rs1 == rs2 then pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param src1 First comparison register
 * @param src2 Second comparison register
 * @param imm Branch target offset
 * @return Pointer to created branch node
 */
IRNode *ir_insert_beq(IR *ir, int src1, int src2, int imm);

/**
 * @brief Insert branch if not equal: if rs1 != rs2 then pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param src1 First comparison register
 * @param src2 Second comparison register
 * @param imm Branch target offset
 * @return Pointer to created branch node
 */
IRNode *ir_insert_bne(IR *ir, int src1, int src2, int imm);

/**
 * @brief Insert branch if less or equal: if rs1 <= rs2 then pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param src1 First comparison register
 * @param src2 Second comparison register
 * @param imm Branch target offset
 * @return Pointer to created branch node
 */
IRNode *ir_insert_ble(IR *ir, int src1, int src2, int imm);

/**
 * @brief Insert branch if less than: if rs1 < rs2 then pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param src1 First comparison register
 * @param src2 Second comparison register
 * @param imm Branch target offset
 * @return Pointer to created branch node
 */
IRNode *ir_insert_blt(IR *ir, int src1, int src2, int imm);

/**
 * @brief Insert branch if greater or equal: if rs1 >= rs2 then pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param src1 First comparison register
 * @param src2 Second comparison register
 * @param imm Branch target offset
 * @return Pointer to created branch node
 */
IRNode *ir_insert_bge(IR *ir, int src1, int src2, int imm);

/**
 * @brief Insert branch if greater than: if rs1 > rs2 then pc ← pc + imm
 * @param ir Pointer to IR structure
 * @param src1 First comparison register
 * @param src2 Second comparison register
 * @param imm Branch target offset
 * @return Pointer to created branch node
 */
IRNode *ir_insert_bgt(IR *ir, int src1, int src2, int imm);

/** @} */

/** @name Function Call Instructions
 * @brief Functions for inserting function calls
 * @{
 */

/**
 * @brief Insert function call
 * @param ir Pointer to IR structure
 * @param label Function name to call
 */
void ir_insert_call(IR *ir, char *label);

/**
 * @brief Insert system call
 * @param ir Pointer to IR structure
 */
void ir_insert_ecall(IR *ir);

/** @} */

#endif /* IR_H */
