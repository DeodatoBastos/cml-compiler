#include "reg_allocation.h"
#include "../global.h"
#include "../utils/bitset.h"
#include "../utils/ir.h"
#include "../utils/stack.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct AdjListNode
 * @brief Node in adjacency list for interference graph
 *
 * Represents a single edge in the interference graph using
 * adjacency list representation. Each node contains the ID
 * of an adjacent vertex and a pointer to the next adjacent vertex.
 */
typedef struct AdjListNode {
    int value;
    struct AdjListNode *next;
} AdjListNode;

/**
 * @struct InterferenceGraph
 * @brief Interference graph for register allocation
 *
 * Represents the interference relationships between virtual registers.
 * Two registers interfere if they are simultaneously live at some
 * program point, meaning they cannot be assigned to the same physical register.
 */
typedef struct InterferenceGraph {
    int num_nodes;
    int *num_neighbors;
    AdjListNode **adj_list;
} InterferenceGraph;

/**
 * @brief Perform liveness analysis on IR code
 *
 * Computes live-in and live-out sets for each instruction using iterative
 * dataflow analysis. A register is "live" at a program point if its current
 * value may be used along some execution path starting from that point.
 *
 * The algorithm works backwards through the code, propagating liveness
 * information until a fixed point is reached. For each instruction:
 * - live_out[i] = union of live_in sets of all successors
 * - live_in[i] = use[i] ∪ (live_out[i] - def[i])
 *
 * Where:
 * - use[i] = registers read by instruction i
 * - def[i] = registers written by instruction i
 *
 * @param ir Pointer to IR structure to analyze
 *
 * @note This function modifies the IR by setting live_in and live_out
 *       bitsets for each instruction node.
 */
static void liveness_analysis(IR *ir) {
    int num_temps = ir->next_temp_reg;
    bool changed = true;

    while (changed) {
        changed = false;
        for (IRNode *node = ir->tail; node != NULL; node = node->prev) {
            if (node->instruction == COMMENT)
                continue;

            // Find the next non-comment instruction (successor)
            IRNode *succ = node->next;
            while (succ != NULL && succ->instruction == COMMENT) {
                succ = succ->next;
            }
            // For jump instructions, successor is the jump target
            if (node->instruction == JUMP) {
                succ = node->target;
            }

            // `out[v]` = U_{w in succ(v)} `in[w]`
            BitSet *new_out = succ == NULL ? NULL : bitset_copy(succ->live_in);

            // Branch instructions have two successors: fall-through and branch target
            if (node->instruction == BEQ || node->instruction == BNE || node->instruction == BLE ||
                node->instruction == BLT || node->instruction == BGE || node->instruction == BGT) {
                if (new_out == NULL && node->target->live_in != NULL)
                    new_out = bitset_copy(node->target->live_in);
                else
                    bitset_union(new_out, node->target->live_in);
            }

            // `in[v]` <-  use(v) U (`out[v]` - def(v))
            BitSet *new_in = new_out != NULL ? bitset_copy(new_out)
                             : ((node->dest > 0) | (node->src1 > 0) | (node->src2 > 0))
                                 ? new_bitset(num_temps)
                                 : NULL;
            if ((node->dest > 0) && (new_out != NULL) && (bitset_test(new_out, node->dest)))
                bitset_clear(new_in, node->dest);
            if (node->src1 > 0)
                bitset_set(new_in, node->src1);
            if (node->src2 > 0)
                bitset_set(new_in, node->src2);

            if (!bitset_equals(node->live_out, new_out) || !bitset_equals(node->live_in, new_in)) {
                changed = true;
                destroy_biset(node->live_out);
                destroy_biset(node->live_in);
                node->live_out = new_out;
                node->live_in = new_in;
            } else {
                destroy_biset(new_out);
                destroy_biset(new_in);
            }
        }
    }
}

/**
 * @brief Create a new interference graph
 *
 * Allocates memory for an interference graph with the specified number
 * of nodes and initializes all adjacency lists to empty.
 *
 * @param n_nodes Number of nodes (virtual registers) in the graph
 * @return Pointer to newly created interference graph
 */
static InterferenceGraph *create_graph(int n_nodes) {
    InterferenceGraph *g = (InterferenceGraph *)malloc(sizeof(InterferenceGraph));
    g->num_nodes = n_nodes;
    g->num_neighbors = (int *)malloc(n_nodes * sizeof(int));
    g->adj_list = (AdjListNode **)malloc(n_nodes * sizeof(AdjListNode *));

    for (int n = 0; n < n_nodes; n++) {
        g->num_neighbors[n] = 0;
        g->adj_list[n] = NULL;
    }

    return g;
}

/**
 * @brief Print interference graph for debugging
 *
 * Outputs the adjacency lists of all nodes in the interference graph
 * to help with debugging and visualization of register conflicts.
 *
 * @param g Pointer to interference graph to print
 */
static void print_graph(InterferenceGraph *g) {
    for (int u = 0; u < g->num_nodes; u++) {
        printf("Neighbors of %d: ", u);
        for (AdjListNode *v = g->adj_list[u]; v != NULL; v = v->next) {
            printf("%d ", v->value);
        }
        printf("\n");
    }
}

/**
 * @brief Add an undirected edge to the interference graph
 *
 * Creates an interference edge between two virtual registers,
 * indicating they cannot be assigned to the same physical register.
 * The edge is added in both directions since interference is symmetric.
 *
 * @param g Pointer to interference graph
 * @param u First virtual register ID
 * @param v Second virtual register ID
 *
 * @note Self-loops (u == v) are ignored since a register doesn't
 *       interfere with itself.
 */
static void add_edge(InterferenceGraph *g, int u, int v) {
    if (u == v)
        return;

    // Add edge u -> v
    AdjListNode *new_u = (AdjListNode *)malloc(sizeof(AdjListNode));
    new_u->value = v;
    new_u->next = g->adj_list[u];
    g->adj_list[u] = new_u;
    g->num_neighbors[u] += 1;

    // Add edge v -> u (undirected graph)
    AdjListNode *new_v = (AdjListNode *)malloc(sizeof(AdjListNode));
    new_v->value = u;
    new_v->next = g->adj_list[v];
    g->adj_list[v] = new_v;
    g->num_neighbors[v] += 1;
}

/**
 * @brief Free memory used by interference graph
 *
 * Deallocates all memory associated with the interference graph
 * including adjacency lists and the graph structure itself.
 *
 * @param g Pointer to interference graph to destroy
 */
static void destroy_graph(InterferenceGraph *g) {
    free(g->num_neighbors);
    for (int u = 0; u < g->num_nodes; u++) {
        AdjListNode *node = g->adj_list[u];
        AdjListNode *next = NULL;
        while (node != NULL) {
            next = node->next;
            free(node);
            node = next;
        }
    }
    free(g->adj_list);
    free(g);
}

/**
 * @brief Build interference graph from IR with liveness information
 *
 * Constructs the interference graph by examining each instruction that
 * defines a register. A register interferes with all registers that are
 * live after the instruction (live_out set), since they cannot share
 * the same physical register.
 *
 * @param ir Pointer to IR structure (must have liveness analysis completed)
 * @return Pointer to constructed interference graph
 */
static InterferenceGraph *build_graph(IR *ir) {
    int num_temps = ir->next_temp_reg;
    InterferenceGraph *g = create_graph(num_temps);
    liveness_analysis(ir);

    for (IRNode *node = ir->head; node != NULL; node = node->next) {
        if (node->dest > 0) {
            for (int j = 1; j < num_temps; j++) {
                if (bitset_test(node->live_out, j)) {
                    add_edge(g, node->dest, j);
                }
            }
        }
    }

    return g;
}

/**
 * @brief Color the interference graph using greedy algorithm
 *
 * Assigns colors (physical registers) to nodes (virtual registers) such that
 * no two adjacent nodes have the same color. Uses a stack-based approach:
 *
 * 1. **Simplification**: Remove nodes with < K neighbors and push onto stack
 * 2. **Spilling**: If no such nodes exist, select node with most neighbors
 * 3. **Coloring**: Pop nodes from stack and assign first available color
 *
 * @param g Pointer to interference graph to color
 * @param num_temps Number of virtual registers (nodes)
 * @param num_colors Number of available colors (physical registers)
 * @return Array mapping virtual register IDs to assigned colors
 *
 * @note If a node cannot be colored (register spilling required),
 *       the function reports an error and sets the global Error flag.
 */
static int *color_graph(InterferenceGraph *g, int num_temps, int num_colors) {
    int *map = (int *)malloc(num_temps * sizeof(int));
    bool *active = (bool *)malloc(num_temps * sizeof(bool));
    for (int i = 0; i < num_temps; i++)
        active[i] = true;

    Stack *stack = s_create();
    int num_nodes = num_temps;

    // Simplification phase: remove nodes and push onto stack
    while (num_nodes > 0) {
        int max_neighbors = -1, sel_node = -1;

        // First, try to find a node with < K neighbors (guaranteed colorable)
        for (int i = 0; i < num_temps; i++) {
            if (active[i] &&
                ((g->num_neighbors[i] > max_neighbors && g->num_neighbors[i] < num_colors))) {
                max_neighbors = g->num_neighbors[i];
                sel_node = i;
            }
        }

        // If no such node exists, select node with most neighbors (spill candidate)
        if (sel_node == -1) {
            for (int i = 0; i < num_temps; i++) {
                if (active[i] && g->num_neighbors[i] > max_neighbors) {
                    max_neighbors = g->num_neighbors[i];
                    sel_node = i;
                }
            }
        }

        s_push(stack, sel_node);
        active[sel_node] = false;

        // remove all nodes linked to the selected
        g->num_neighbors[sel_node] = 0;
        for (AdjListNode *node = g->adj_list[sel_node]; node != NULL; node = node->next) {
            g->num_neighbors[node->value]--;
        }
        num_nodes--;
    }

    // Coloring phase: pop nodes from stack and assign colors
    while (!s_empty(stack)) {
        bool found = false;
        int v = s_top(stack);
        for (int color = 0; color < num_colors; color++) {
            bool available = true;

            // Check if any active neighbor already uses this color
            for (AdjListNode *u = g->adj_list[v]; u != NULL; u = u->next) {
                available = available && !(active[u->value] && map[u->value] == color);

                if (!available)
                    break;
            }

            // If color is available, assign it
            if (available) {
                map[v] = color;
                found = true;
                break;
            }
        }

        // If no color available, register spilling is required
        if (!found) {
            fprintf(listing,
                    "\033[1;31mFatal Error\033[0m: %d registers are not enough, must spill\n",
                    num_colors);
            Error = true;
            break;
        }

        active[v] = true;
        s_pop(stack);
    }

    s_destroy(stack);
    free(active);

    return map;
}

int *allocate_registers(IR *ir) {
    int num_temps = ir->next_temp_reg;
    InterferenceGraph *g = build_graph(ir);
    // print_graph(g);
    int *color_map = color_graph(g, num_temps, K);

    // printf("Color map: ");
    // for (int i = 0; i < num_temps; i++) {
    //     printf("%d ", color_map[i]);
    // }
    // printf("\n");

    destroy_graph(g);

    return color_map;
}
