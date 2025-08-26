#include "queue.h"

#include <assert.h>
#include <stdlib.h>

typedef struct node QNode;

struct node {
    QNode *next;
    void *element;
};

struct queue {
    QNode *begin;
    QNode *end;
    int size;
};

Queue *q_create() {
    Queue *queue;
    queue = (Queue *)malloc(sizeof(Queue));
    assert(queue != NULL);

    queue->begin = NULL;
    queue->end = NULL;
    queue->size = 0;

    return queue;
}

void q_destroy(Queue *queue) {
    while (!q_empty(queue))
        q_pop(queue);

    free(queue);
}

void q_push(Queue *queue, void *element) {
    if (q_empty(queue)) {
        queue->end = (QNode *)malloc(sizeof(QNode));
        assert(queue->end != NULL);

        queue->end->next = NULL;
        queue->end->element = element;
        queue->begin = queue->end;

        return;
    }

    QNode *new_node = (QNode *)malloc(sizeof(QNode));

    queue->end->next = new_node;
    new_node->next = NULL;
    new_node->element = element;
    queue->end = new_node;
    queue->size++;
}

void q_pop(Queue *queue) {
    if (q_empty(queue))
        return;

    QNode *auxNode = queue->begin;
    queue->begin = queue->begin->next;

    if (queue->begin == NULL)
        queue->end = queue->begin;

    queue->size--;
    free(auxNode);
}

void *q_front(const Queue *queue) { return queue->begin->element; }

void *q_back(const Queue *queue) { return queue->end->element; }

bool q_empty(const Queue *queue) { return queue->begin == NULL; }

int q_size(const Queue *queue) { return queue->size; }
