#include "cgen.h"
#include "../parser.tab.h"
#include "../utils/ast.h"
#include "../utils/ir.h"
#include "../utils/symtab.h"
#include <stdio.h>
#include <string.h>

// Forward declarations for static helper functions
static void gen_code(ASTNode *node, IR *ir);
static int calculate_local_size(ASTNode *node);
static IRNode *gen_condition(ASTNode *node, IR *ir);

/**
 * @brief A global pointer to the end label of the current function being generated.
 * This is used by 'return' statements to know where the function's epilogue is.
 */
static ASTNode *func;
// static IRNode *func_end = NULL;

IR *gen_ir(ASTNode *tree) {
    IR *ir = new_ir();

    ir_insert_comment(ir, "program entry: call main");
    ir_insert_call(ir, "main");
    ir_insert_comment(ir, "syscall Exit (a7 = 10)");
    ir_insert_addi(ir, A7_REGISTER, X0_REGISTER, 10);
    ir_insert_ecall(ir);
    gen_code(tree, ir);

    return ir;
}

/**
 * @brief Main entry point for the code generation phase.
 * It initializes the IR, creates a program header to call 'main' and then exit,
 * and then starts the recursive traversal of the AST.
 *
 * @param tree The root of the AST.
 * @return A pointer to the generated IR.
 */
void gen_code(ASTNode *node, IR *ir) {
    if (node == NULL)
        return;

    switch (node->node_kind) {
    case Stmt:
        switch (node->kind.stmt) {
        case Compound:
            ir_insert_comment(ir, "enter compound block");
            gen_code(node->child[0], ir);
            gen_code(node->child[1], ir);
            ir_insert_comment(ir, "leave compound block");
            break;

        case Assign: {
            gen_code(node->child[1], ir);
            int rs2 = node->child[1]->temp_reg;
            ASTNode *var_node = node->child[0];
            BucketList *bucket = st_lookup(var_node->attr.name, var_node->scope);

            // TODO: should I load bucket->address/offset into a register?
            // (to big for 12 bits [-2048,2047] ?)
            if (var_node->scope == 0) {
                int base_addr_reg = register_new_temp(ir);
                ir_insert_li(ir, base_addr_reg, bucket->address);
                if (var_node->child[0] == NULL) {
                    ir_insert_comment(ir, "store: mem[addr] <- rs2");
                    // ir_insert_store(ir, rs2, bucket->address, X0_REGISTER);
                    ir_insert_store(ir, rs2, 0, base_addr_reg);
                } else {
                    gen_code(var_node->child[0], ir);
                    int idx_reg = var_node->child[0]->temp_reg;
                    int offset_reg = register_new_temp(ir);
                    ir_insert_slli(ir, offset_reg, idx_reg, 2); // 4-byte integer
                    // ir_insert_comment(ir, "store: mem[addr+rs1] <- rs2");
                    // ir_insert_store(ir, rs2, bucket->address, offset_reg);
                    int addr_reg = register_new_temp(ir);
                    ir_insert_add(ir, addr_reg, base_addr_reg, offset_reg);
                    ir_insert_comment(ir, "store: mem[rs1] <- rs2");
                    ir_insert_store(ir, rs2, 0, addr_reg);
                }
            } else {
                if (var_node->child[0] == NULL) {
                    ir_insert_comment(ir, "store: mem[offset+fp] <- rs2");
                    ir_insert_store(ir, rs2, bucket->offset, FP_REGISTER);
                } else {
                    gen_code(var_node->child[0], ir);
                    int idx_reg = var_node->child[0]->temp_reg;
                    int offset_reg = register_new_temp(ir);
                    ir_insert_slli(ir, offset_reg, idx_reg, 2); // 4-byte integer
                    int addr_reg = register_new_temp(ir);

                    if (bucket->offset > 0) { // param var
                        ir_insert_comment(ir, "load address: rd <- mem[offset+fp]");
                        ir_insert_load(ir, addr_reg, bucket->offset, FP_REGISTER);
                        ir_insert_add(ir, addr_reg, addr_reg, offset_reg);
                        ir_insert_comment(ir, "store: mem[offset+rs1] <- rs2");
                        ir_insert_store(ir, rs2, 0, addr_reg);
                    } else { // local var
                        ir_insert_add(ir, addr_reg, FP_REGISTER, offset_reg);
                        ir_insert_comment(ir, "store: mem[offset+rs1] <- rs2");
                        ir_insert_store(ir, rs2, bucket->offset, addr_reg);
                    }
                }
            }
            break;
        }

        case If: {
            int n_if = register_new_if(ir);
            IRNode *end_if = NULL;
            char end_if_label[50];
            snprintf(end_if_label, sizeof(end_if_label), "end_if_%d", n_if);
            IRNode *cond = gen_condition(node->child[0], ir);

            ir_insert_comment(ir, "then-block begin");
            gen_code(node->child[1], ir);
            ir_insert_comment(ir, "then-block end");

            // uncoditionally jump to the end
            if (node->child[2] != NULL) {
                char end_else_label[50];
                snprintf(end_else_label, sizeof(end_else_label), "end_else_%d", n_if);

                ir_insert_comment(ir, "goto end-else");
                IRNode *jump_else = ir_insert_jump(ir, end_else_label);
                end_if = ir_insert_label(ir, end_if_label);
                ir_insert_comment(ir, "else-block begin");
                gen_code(node->child[2], ir);
                ir_insert_comment(ir, "else-block end");
                IRNode *end_else = ir_insert_label(ir, end_else_label);

                // goto end_else
                // jump_else->imm = end_else->address - jump_else->address;
                // jump_else->comment = end_else->comment;
                jump_else->target = end_else;
            } else {
                end_if = ir_insert_label(ir, end_if_label);
            }

            // goto end_if
            // cond->imm = end_if->address - cond->address;
            cond->target = end_if;
            cond->comment = strdup(end_if->comment);
            break;
        }

        case While: {
            int n_while = register_new_while(ir);
            IRNode *start_while = NULL, *end_while = NULL;
            char start_label[50], end_label[50];
            snprintf(start_label, sizeof(start_label), "start_while_%d", n_while);
            snprintf(end_label, sizeof(end_label), "end_while_%d", n_while);

            ir_insert_comment(ir, "while begin");
            start_while = ir_insert_label(ir, start_label);
            IRNode *comp = gen_condition(node->child[0], ir);

            ir_insert_comment(ir, "while-body begin");
            gen_code(node->child[1], ir);
            ir_insert_comment(ir, "while-body end");

            ir_insert_comment(ir, "goto while-begin");
            IRNode *jump_start_while = ir_insert_jump(ir, start_label);
            end_while = ir_insert_label(ir, end_label);
            ir_insert_comment(ir, "while end");

            // goto to the next instrcution after the end_while
            // comp->imm = end_while->address - comp->address;
            comp->target = end_while;
            comp->comment = strdup(end_while->comment);
            // goto condition check
            // jump_start_while->imm = start_while->address - jump_start_while->address;
            // jump_start_while->comment = jump_start_while->comment;
            jump_start_while->target = start_while;
            break;
        }

        case Return: {
            if (node->child[0] != NULL) {
                gen_code(node->child[0], ir);
                ir_insert_comment(ir, "a0 <- rs1");
                ir_insert_mov(ir, A0_REGISTER, node->child[0]->temp_reg);
            }
            ir_insert_comment(ir, "jump to function epilogue");
            // ir_insert_rel_jump(ir, func_end);
            char label[256];
            snprintf(label, sizeof(label), "end_%s", func->attr.name);
            ir_insert_jump(ir, label);
            break;
        }

        case Read: {
            ir_insert_comment(ir, "syscall ReadInt (a7 = 5)");
            ir_insert_addi(ir, A7_REGISTER, X0_REGISTER, 5);
            ir_insert_ecall(ir);

            ASTNode *var_node = node->child[0];
            BucketList *bucket = st_lookup(var_node->attr.name, var_node->scope);

            // TODO: should I load bucket->address/offset into a register?
            // (to big for 12 bits [-2048,2047] ?)
            if (var_node->scope == 0) {
                int base_addr_reg = register_new_temp(ir);
                ir_insert_li(ir, base_addr_reg, bucket->address);
                if (var_node->child[0] == NULL) {
                    ir_insert_comment(ir, "store: mem[addr] <- a0");
                    // ir_insert_store(ir, A0_REGISTER, bucket->address, X0_REGISTER);
                    ir_insert_store(ir, A0_REGISTER, 0, base_addr_reg);
                } else {
                    gen_code(var_node->child[0], ir);
                    int idx_reg = var_node->child[0]->temp_reg;
                    int offset_reg = register_new_temp(ir);
                    ir_insert_slli(ir, offset_reg, idx_reg, 2); // 4-byte integer
                    // ir_insert_comment(ir, "store: mem[addr+rs1] <- rs2");
                    // ir_insert_store(ir, A0_REGISTER, bucket->address, offset_reg);
                    int addr_reg = register_new_temp(ir);
                    ir_insert_add(ir, addr_reg, base_addr_reg, offset_reg);
                    ir_insert_comment(ir, "store: mem[rs1] <- a0");
                    ir_insert_store(ir, A0_REGISTER, 0, addr_reg);
                }
            } else {
                if (var_node->child[0] == NULL) {
                    ir_insert_comment(ir, "store: mem[offset+fp] <- a0");
                    ir_insert_store(ir, A0_REGISTER, bucket->offset, FP_REGISTER);
                } else {
                    gen_code(var_node->child[0], ir);
                    int idx_reg = var_node->child[0]->temp_reg;
                    int offset_reg = register_new_temp(ir);
                    ir_insert_slli(ir, offset_reg, idx_reg, 2); // 4-byte integer
                    int addr_reg = register_new_temp(ir);

                    if (bucket->offset > 0) { // param var
                        ir_insert_comment(ir, "load address: rd <- mem[offset+fp]");
                        ir_insert_load(ir, addr_reg, bucket->offset, FP_REGISTER);
                        ir_insert_add(ir, addr_reg, addr_reg, offset_reg);
                        ir_insert_comment(ir, "store: mem[offset+rs1] <- a0");
                        ir_insert_store(ir, A0_REGISTER, 0, addr_reg);
                    } else { // local var
                        ir_insert_add(ir, addr_reg, FP_REGISTER, offset_reg);
                        ir_insert_comment(ir, "store: mem[offset+rs1] <- a0");
                        ir_insert_store(ir, A0_REGISTER, bucket->offset, addr_reg);
                    }
                }
            }
            break;
        }

        case Write: {
            gen_code(node->child[0], ir);
            ir_insert_comment(ir, "syscall PrintInt (a7 = 1, a0 = rs1)");
            ir_insert_addi(ir, A7_REGISTER, X0_REGISTER, 1);
            ir_insert_mov(ir, A0_REGISTER, node->child[0]->temp_reg);
            ir_insert_ecall(ir);
            ir_insert_addi(ir, A7_REGISTER, X0_REGISTER, 11);
            ir_insert_li(ir, A0_REGISTER, 10);
            ir_insert_ecall(ir);
            break;
        }
        }
        break;

    case Expr:
        switch (node->kind.expr) {
        case Const:
            node->temp_reg = register_new_temp(ir);
            ir_insert_li(ir, node->temp_reg, node->attr.val);
            break;

        case Op:
            if (node->type == Integer) {
                gen_code(node->child[0], ir);
                gen_code(node->child[1], ir);

                int rs1 = node->child[0]->temp_reg;
                int rs2 = node->child[1]->temp_reg;
                int rd = register_new_temp(ir);
                node->temp_reg = rd;

                switch (node->attr.op) {
                case PLUS:
                    ir_insert_add(ir, rd, rs1, rs2);
                    break;
                case MINUS:
                    ir_insert_sub(ir, rd, rs1, rs2);
                    break;
                case TIMES:
                    ir_insert_mul(ir, rd, rs1, rs2);
                    break;
                case OVER:
                    ir_insert_div(ir, rd, rs1, rs2);
                    break;
                case MOD:
                    ir_insert_rem(ir, rd, rs1, rs2);
                    break;
                default:
                    break;
                }
                break;
            }
            break;

        case Var:
        case Arr: {
            BucketList *bucket = st_lookup(node->attr.name, node->scope);
            int value_reg = register_new_temp(ir);

            // TODO: should I load bucket->address/offset into a register?
            // (to big for 12 bits [-2048,2047] ?)
            if (node->scope == 0) {
                int base_addr_reg = register_new_temp(ir);
                ir_insert_li(ir, base_addr_reg, bucket->address);
                if (node->child[0] == NULL) {
                    ir_insert_comment(ir, "load: rd <- mem[addr]");
                    // ir_insert_load(ir, value_reg, bucket->address, X0_REGISTER);
                    ir_insert_load(ir, value_reg, 0, base_addr_reg);
                } else {
                    gen_code(node->child[0], ir);
                    int idx_reg = node->child[0]->temp_reg;
                    int offset_reg = register_new_temp(ir);
                    ir_insert_slli(ir, offset_reg, idx_reg, 2); // 4-byte integer
                    // ir_insert_comment(ir, "load: rd <- mem[addr+rs1]");
                    // ir_insert_load(ir, value_reg, bucket->address, offset_reg);
                    int addr_reg = register_new_temp(ir);
                    ir_insert_add(ir, addr_reg, base_addr_reg, offset_reg);
                    ir_insert_comment(ir, "load: rd <- mem[rs1]");
                    ir_insert_load(ir, value_reg, 0, addr_reg);
                }
            } else {
                if (node->child[0] == NULL) {
                    ir_insert_comment(ir, "load: rd <- mem[offset+fp]");
                    ir_insert_load(ir, value_reg, bucket->offset, FP_REGISTER);
                } else {
                    gen_code(node->child[0], ir);
                    int idx_reg = node->child[0]->temp_reg;
                    int offset_reg = register_new_temp(ir);
                    ir_insert_slli(ir, offset_reg, idx_reg, 2); // 4-byte integer
                    int addr_reg = register_new_temp(ir);

                    if (bucket->offset > 0) { // param var
                        ir_insert_comment(ir, "load address: rd <- mem[offset+fp]");
                        ir_insert_load(ir, addr_reg, bucket->offset, FP_REGISTER);
                        ir_insert_add(ir, addr_reg, addr_reg, offset_reg);
                        ir_insert_comment(ir, "load: rd <- mem[offset+rs1]");
                        ir_insert_load(ir, value_reg, 0, addr_reg);
                    } else { // local var
                        ir_insert_add(ir, addr_reg, FP_REGISTER, offset_reg);
                        ir_insert_comment(ir, "load: rd <- mem[offset+rs1]");
                        ir_insert_load(ir, value_reg, bucket->offset, addr_reg);
                    }
                }
            }
            node->temp_reg = value_reg;

            break;
        }

        case FuncDecl: {
            ir_insert_comment(ir, "func begin");
            ir_insert_label(ir, node->attr.name);

            // store preserved registers
            ir_insert_comment(ir, "func prologue");
            ir_insert_addi(ir, SP_REGISTER, SP_REGISTER, -8);
            ir_insert_store(ir, RA_REGISTER, 4, SP_REGISTER);
            ir_insert_store(ir, FP_REGISTER, 0, SP_REGISTER);
            ir_insert_mov(ir, FP_REGISTER, SP_REGISTER);

            // Pre-pass to calculate stack size
            int local_size = calculate_local_size(node->child[1]);
            if (local_size > 0) {
                ir_insert_addi(ir, SP_REGISTER, SP_REGISTER, -local_size);
            }

            // Func Body
            // IRNode *old_func_end = func_end;
            // func_end = new_ir_node(NOP);
            ASTNode *old_func = func;
            func = node;
            ir_insert_comment(ir, "func body");
            gen_code(node->child[1], ir);

            // Func Epilogue
            ir_insert_comment(ir, "func epilogue");
            char label[256];
            snprintf(label, sizeof(label), "end_%s", node->attr.name);
            ir_insert_label(ir, label);
            // ir_insert_node(ir, func_end);

            // restore registers
            ir_insert_mov(ir, SP_REGISTER, FP_REGISTER);
            ir_insert_load(ir, RA_REGISTER, 4, SP_REGISTER);
            ir_insert_load(ir, FP_REGISTER, 0, SP_REGISTER);
            ir_insert_addi(ir, SP_REGISTER, SP_REGISTER, 8);

            // return to caller
            ir_insert_jump_reg(ir, RA_REGISTER);

            // func_end = old_func_end;
            func = old_func;
            break;
        }

        case FuncCall: {
            ASTNode *arg = node->child[0];
            int arg_count = 0, count = 0;
            ir_insert_comment(ir, "push arguments");

            while (arg != NULL) {
                arg_count++;
                arg = arg->sibling;
            }
            arg = node->child[0];
            if (arg_count > 0)
                ir_insert_addi(ir, SP_REGISTER, SP_REGISTER, -arg_count * 4);

            while (arg != NULL) {
                ASTNode *next_arg = arg->sibling; // avoid generation of args code multiple times
                arg->sibling = NULL;

                if (arg->kind.expr == Arr && arg->child[0] == NULL) {
                    ir_insert_comment(ir, "push array arg");
                    BucketList *bucket = st_lookup(arg->attr.name, arg->scope);
                    int addr_reg = register_new_temp(ir);
                    if (arg->scope == 0) {
                        ir_insert_comment(ir, "load global adddres: rd <- addr");
                        ir_insert_li(ir, addr_reg, bucket->address);
                    } else {
                        if (bucket->node->kind.expr == ParamArr) {
                            ir_insert_comment(ir, "load address from arg: rd <- mem[offset+fp]");
                            ir_insert_load(ir, addr_reg, bucket->offset, FP_REGISTER);
                        } else {
                            ir_insert_comment(ir, "load local address: rd <- fp+offset");
                            ir_insert_addi(ir, addr_reg, FP_REGISTER, bucket->offset);
                        }
                    }
                    arg->temp_reg = addr_reg;
                } else {
                    ir_insert_comment(ir, "push other arg");
                    gen_code(arg, ir);
                }

                arg->sibling = next_arg;

                ir_insert_store(ir, arg->temp_reg, count * 4, SP_REGISTER);
                count++;
                arg = arg->sibling;
            }

            // call the function
            ir_insert_call(ir, node->attr.name);

            if (arg_count > 0) {
                ir_insert_comment(ir, "restore stack");
                ir_insert_addi(ir, SP_REGISTER, SP_REGISTER, arg_count * 4);
            }

            node->temp_reg = register_new_temp(ir);
            ir_insert_comment(ir, "rs1 <- a0");
            ir_insert_mov(ir, node->temp_reg, A0_REGISTER);
            break;
        }

        case VarDecl:
        case ArrDecl:
        case ParamVar:
        case ParamArr:
            break;
        }
        break;
    }

    gen_code(node->sibling, ir);
}

/**
 * @brief Recursively traverses the AST to generate IR code for each node.
 *
 * @param node The current AST node to process.
 * @param ir The IR structure to append new instructions to.
 */
static int calculate_local_size(ASTNode *node) {
    if (node == NULL)
        return 0;

    int size = 0;
    if (node->node_kind == Stmt && node->kind.stmt == Compound) {
        ASTNode *decl = node->child[0];
        while (decl != NULL) {
            if (decl->node_kind == Expr) {
                if (decl->kind.expr == VarDecl) {
                    size += 4;
                } else if (decl->kind.expr == ArrDecl) {
                    size += decl->child[0]->attr.val * 4;
                }
            }
            decl = decl->sibling;
        }
        size += calculate_local_size(node->child[1]);
    } else {
        for (int i = 0; i < MAXCHILDREN; i++)
            size += calculate_local_size(node->child[i]);
        size += calculate_local_size(node->sibling);
    }

    return size;
}

/**
 * @brief Generates the conditional branch instruction for an 'if' or 'while' statement.
 *
 * @param node The expression node representing the condition.
 * @param ir The IR structure.
 * @return A pointer to the generated branch instruction, to be used for backpatching.
 */
static IRNode *gen_condition(ASTNode *node, IR *ir) {
    gen_code(node->child[0], ir);
    gen_code(node->child[1], ir);

    int rs1 = node->child[0]->temp_reg;
    int rs2 = node->child[1]->temp_reg;
    IRNode *comp = NULL;

    switch (node->attr.op) {
    case EQ:
        ir_insert_comment(ir, "cmp: if (rs1 != rs2 goto false)");
        comp = ir_insert_bne(ir, rs1, rs2, 0);
        break;
    case NE:
        ir_insert_comment(ir, "cmp: if (rs1 == rs2 goto false)");
        comp = ir_insert_beq(ir, rs1, rs2, 0);
        break;
    case LT:
        ir_insert_comment(ir, "cmp: if (rs1 >= rs2 goto false)");
        comp = ir_insert_bge(ir, rs1, rs2, 0);
        break;
    case LE:
        ir_insert_comment(ir, "cmp: if (rs1 > rs2 goto false)");
        comp = ir_insert_bgt(ir, rs1, rs2, 0);
        break;
    case GT:
        ir_insert_comment(ir, "cmp: if (rs1 <= rs2 goto false)");
        comp = ir_insert_ble(ir, rs1, rs2, 0);
        break;
    case GE:
        ir_insert_comment(ir, "cmp: if (rs1 < rs2 goto false)");
        comp = ir_insert_blt(ir, rs1, rs2, 0);
        break;
    }

    return comp;
}
