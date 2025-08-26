#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct queue Queue;

Queue *q_create();

void q_destroy(Queue *queue);

void q_push(Queue *queue, void *element);

void q_pop(Queue *queue);

void *q_front(const Queue *queue);

void *q_back(const Queue *queue);

bool q_empty(const Queue *queue);

int q_size(const Queue *queue);

#endif // QUEUE_H
