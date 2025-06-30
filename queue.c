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
};

Queue *q_create() {
    Queue *queue;
    queue = (Queue *)malloc(sizeof(Queue));
    assert(queue != NULL);

    queue->begin = NULL;
    queue->end = NULL;

    return queue;
}

void q_destroy(Queue *queue) {
    while (!q_isEmpty(queue))
        q_pop(queue);

    free(queue);
}

void q_push(Queue *queue, void *element) {
    if (q_isEmpty(queue)) {
        queue->end = (QNode *)malloc(sizeof(QNode));
        assert(queue->end != NULL);

        queue->end->next = NULL;
        queue->end->element = element;
        queue->begin = queue->end;

        return;
    }

    QNode *newNode = (QNode *)malloc(sizeof(QNode));

    queue->end->next = newNode;
    newNode->next = NULL;
    newNode->element = element;
    queue->end = newNode;
}

void q_pop(Queue *queue) {
    if (q_isEmpty(queue))
        return;

    QNode *auxNode = queue->begin;
    queue->begin = queue->begin->next;

    if (queue->begin == NULL)
        queue->end = queue->begin;

    free(auxNode);
}

void *q_front(const Queue *queue) { return queue->begin->element; }

void *q_back(const Queue *queue) { return queue->end->element; }

bool q_isEmpty(const Queue *queue) { return queue->begin == NULL; }
