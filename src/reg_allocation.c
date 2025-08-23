#include "reg_allocation.h"
#include "bitset.h"
#include "global.h"
#include "ir.h"
#include "stack.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define K 4
// static const int PHYSICAL_REGS[K] = {-5, -6, -7, -28, -29, -30, -31}; // t0-t6

typedef struct AdjListNode {
    int value;
    struct AdjListNode *next;
} AdjListNode;

typedef struct InterferenceGraph {
    int num_nodes;
    int *num_neighbors;
    bool **adj_matrix;
    AdjListNode **adj_list;
} InterferenceGraph;

// static void compute_use_def(IRNode *node, BitSet *use, BitSet *def) {
//     if (node->src1 > 0)
//         bitset_set(use, node->src1);
//     if (node->src2 > 0)
//         bitset_set(use, node->src2);
//     if (node->dest > 0)
//         bitset_set(def, node->dest);
// }

static void liveness_analysis(IR *ir) {
    int num_temps = ir->next_temp_reg;
    bool changed = true;
    while (changed) {
        changed = false;
        for (IRNode *node = ir->tail; node != NULL; node = node->prev) {
            if (node->instruction == COMMENT)
                continue;

            IRNode *succ = node->next;
            // Successor of Jump is the target
            if (node->instruction == JUMP || node->instruction == RELATIVE_JUMP) {
                succ = node->target;
            }

            // `out[v]` = U_{w in succ(v)} `in[w]`
            BitSet *new_out = succ == NULL ? NULL : bitset_copy(succ->live_in);
            // Branches have 2 successors (next instruction and the jump instruction)
            if (node->instruction == BEQ || node->instruction == BNE || node->instruction == BLE ||
                node->instruction == BLT || node->instruction == BGE || node->instruction == BGT) {
                if (new_out == NULL && node->target->live_in != NULL)
                    new_out = bitset_copy(node->target->live_in);
                else
                    bitset_union(new_out, node->target->live_in);
            }

            // BitSet *use = new_bitset(num_temps);
            // BitSet *def = new_bitset(num_temps);
            // compute_use_def(node, use, def);
            // // `in[v]` <-  use(v) U (`out[v]` - def(v))
            // BitSet *new_in = bitset_copy(new_out);
            // bitset_union(new_in, use);
            // bitset_diff(new_in, def);

            // `in[v]` <-  use(v) U (`out[v]` - def(v))
            BitSet *new_in = new_out == NULL ? new_bitset(num_temps) : bitset_copy(new_out);
            if (node->dest > 0)
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

static InterferenceGraph *create_graph(int n_nodes) {
    InterferenceGraph *g = (InterferenceGraph *)malloc(sizeof(InterferenceGraph));
    g->num_nodes = n_nodes;
    g->num_neighbors = (int *)malloc(n_nodes * sizeof(int));
    g->adj_matrix = (bool **)malloc(n_nodes * sizeof(bool *));
    g->adj_list = (AdjListNode **)malloc(n_nodes * sizeof(AdjListNode *));

    for (int n = 0; n < n_nodes; n++) {
        g->num_neighbors[n] = 0;
        g->adj_matrix[n] = (bool *)calloc(n_nodes, sizeof(bool));
        g->adj_list[n] = NULL;
        // g->adj_list[n] = (AdjListNode *)malloc(n_nodes * sizeof(AdjListNode));
        // g->adj_list[n]->next = NULL;
        // g->adj_list[n]->value = -1;
    }

    return g;
}

static void print_graph(InterferenceGraph *g) {
    for (int u = 0; u < g->num_nodes; u++) {
        printf("Neighbors of %d: ", u);
        for (AdjListNode *v = g->adj_list[u]; v != NULL; v = v->next) {
            printf("%d ", v->value);
        }
        printf("\n");
    }
}

static void add_edge(InterferenceGraph *g, int u, int v) {
    if (u == v || g->adj_matrix[u][v])
        return;

    AdjListNode *new_u = (AdjListNode *)malloc(sizeof(AdjListNode));
    new_u->value = v;
    new_u->next = g->adj_list[u];
    g->adj_list[u] = new_u;
    g->num_neighbors[u] += 1;
    g->adj_matrix[u][v] = true;

    AdjListNode *new_v = (AdjListNode *)malloc(sizeof(AdjListNode));
    new_v->value = u;
    new_v->next = g->adj_list[v];
    g->adj_list[v] = new_v;
    g->num_neighbors[v] += 1;
    g->adj_matrix[v][u] = true;
}

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
        free(g->adj_matrix[u]);
    }
    free(g->adj_matrix);
    free(g->adj_list);
    free(g);
}

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

static int *color_graph(InterferenceGraph *g, int num_temps, int num_colors) {
    int *map = (int *)malloc(num_temps * sizeof(int));
    bool *active = (bool *)malloc(num_temps * sizeof(bool));
    for (int i = 0; i < num_temps; i++)
        active[i] = true;

    Stack *stack = s_create();
    int num_nodes = num_temps;
    while (num_nodes > 0) {
        int max_neighbors_under_k = 0, sel_node = -1;
        for (int i = 0; i < num_temps; i++) {
            if (active[i] && (sel_node == -1 || g->num_neighbors[i] > max_neighbors_under_k)) {
                max_neighbors_under_k = g->num_neighbors[i];
                sel_node = i;
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

    // coloring
    while (!s_isEmpty(stack)) {
        // iterate colors
        //   iterate neighbors
        //     neighbor active and has color: color unavailable
        //   if color is available: Mark color
        //   activate this node
        bool found = false;
        for (int color = 0; color < num_colors; color++) {
            bool color_available = true;

            for (AdjListNode *node = g->adj_list[s_top(stack)]; node != NULL; node = node->next) {
                color_available =
                    color_available && !(active[node->value] && map[node->value] == color);
            }

            if (color_available) {
                map[s_top(stack)] = color;
                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(listing, "Fatal: %d registers are not enough, must spill\n", num_colors);
            break;
        }

        active[s_top(stack)] = 1;
        s_pop(stack);
    }

    s_destroy(stack);
    free(active);

    return map;
}

int *allocate_registers(IR *ir) {
    int num_temps = ir->next_temp_reg;
    InterferenceGraph *g = build_graph(ir);
    print_graph(g);
    int *color_map = color_graph(g, num_temps, K);

    printf("Color map: ");
    for (int i = 0; i < num_temps; i++) {
        printf("%d ", color_map[i]);
    }
    printf("\n");

    destroy_graph(g);

    return color_map;
}
