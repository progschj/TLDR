#ifndef QUEUE_H
#define QUEUE_H

typedef struct queue_t {
    int front, back;
    int capacity, element_size;
    char *data;
} queue;

queue queue_create(int element_size);

void queue_push(queue *q, const void *elem);

void queue_pop(queue *q);

void* queue_front(queue *q);

int queue_empty(queue *q);

void queue_destroy(queue q);

#endif
