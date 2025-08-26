#include "stack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node SNode;

struct stack {
    SNode *top;
    int size;
};

struct node {
    SNode *below;
    int element;
};

Stack *s_create() {
    Stack *stack;
    stack = (Stack *)malloc(sizeof(Stack));
    assert(stack != NULL);
    stack->top = NULL;
    stack->size = 0;
    return stack;
}

void s_destroy(Stack *stack) {
    while (stack->top != NULL) {
        s_pop(stack);
    }
    free(stack);
}

bool s_empty(const Stack *stack) { return stack->size == 0; }

int s_top(const Stack *stack) { return stack->top->element; }

void s_push(Stack *stack, int element) {
    SNode *newNode;
    newNode = (SNode *)malloc(sizeof(SNode));
    assert(newNode != NULL);

    newNode->element = element;
    newNode->below = stack->top;
    stack->top = newNode;
    stack->size++;
}

void s_pop(Stack *stack) {
    if (stack->top == NULL)
        return;

    SNode *auxNode;
    auxNode = stack->top;
    stack->top = stack->top->below;
    stack->size--;
    free(auxNode);
}

void s_print_top_down(Stack *stack) {
    printf("Stack (%d): ", stack->size);
    Stack *auxStack;
    auxStack = s_create();

    while (!s_empty(stack)) {
        s_push(auxStack, s_top(stack));
        printf("%d ", s_top(stack));
        s_pop(stack);
    }

    printf("\n");
    while (!s_empty(auxStack)) {
        s_push(stack, s_top(auxStack));
        s_pop(auxStack);
    }

    s_destroy(auxStack);
}
