#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"

typedef struct node StackNode;

struct stack {
    StackNode *top;
    int size;
};

struct node {
    StackNode *below;
    int element;
};


Stack *stack_create() {
    Stack *stack;
    stack = (Stack*) malloc(sizeof(Stack));
    assert(stack != NULL);
    stack->top = NULL;
    stack->size = 0;
    return stack;
}

void stack_destroy(Stack *stack) {
    while (stack->top != NULL) {
        stack_pop(stack);
    }
    free(stack);
}

bool stack_isEmpty(const Stack *stack) {
    return stack->size == 0;
}

int stack_top(const Stack *stack) {
    return stack->top->element;
}

void stack_push(Stack *stack, int element) {
    StackNode *newNode;
    newNode = (StackNode*) malloc(sizeof(StackNode));
    assert(newNode != NULL);

    newNode->element = element;
    newNode->below = stack->top;
    stack->top = newNode;
    stack->size++;
}

void stack_pop(Stack *stack) {
    if (stack->top == NULL) return;

    StackNode *auxNode;
    auxNode = stack->top;
    stack->top = stack->top->below;
    stack->size--;
    free(auxNode);
}

void stack_printTopDown(Stack *stack) {
    printf("Stack (%d): ", stack->size);
    Stack *auxStack;
    auxStack = stack_create();

    while (!stack_isEmpty(stack)) {
        stack_push(auxStack, stack_top(stack));
        printf("%d ", stack_top(stack));
        stack_pop(stack);
    }

    printf("\n");
    while (!stack_isEmpty(auxStack)) {
        stack_push(stack, stack_top(auxStack));
        stack_pop(auxStack);
    }

    stack_destroy(auxStack);
}
