#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

typedef struct stack Stack;

Stack *stack_create();

void stack_destroy(Stack *stack);

bool stack_isEmpty(const Stack *stack);

int stack_top(const Stack *stack);

int stack_size(const Stack *stack);

void stack_push(Stack *stack, int element);

void stack_pop(Stack *stack);

void stack_printTopDown(Stack *stack);

#endif // STACK_H
