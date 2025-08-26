#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

typedef struct stack Stack;

Stack *s_create();

void s_destroy(Stack *stack);

bool s_empty(const Stack *stack);

int s_top(const Stack *stack);

int s_size(const Stack *stack);

void s_push(Stack *stack, int element);

void s_pop(Stack *stack);

void s_print_top_down(Stack *stack);

#endif // STACK_H
