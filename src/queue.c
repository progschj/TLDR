#include <queue.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

queue queue_create(int element_size) {
    queue q = {0, 0, 4, element_size, malloc(4*element_size)};
    return q;
}

static void queue_increase_capacity(queue *q) {
    int newcapacity = 3*q->capacity/2;
    char *newdata = malloc(newcapacity*q->element_size);
    int tail = q->capacity - q->front;
    memcpy(newdata, q->data + q->front*q->element_size, tail*q->element_size);
    memcpy(newdata + tail*q->element_size, q->data, q->back*q->element_size);
    q->data = newdata;
    q->back = q->back + tail;
    q->front = 0;
    q->capacity = newcapacity;
}

void queue_push(queue *q, const void *elem) {
    memcpy(q->data + q->element_size*q->back, elem, q->element_size);
    q->back = (q->back+1) % q->capacity;
    if(q->front == q->back) {
        queue_increase_capacity(q);
    }
}

void queue_pop(queue *q) {
    q->front = (q->front+1) % q->capacity;
}

void* queue_front(queue *q) {
    return q->data + q->element_size*q->front;
}

int queue_empty(queue *q) {
    return q->front == q->back;
}

void queue_destroy(queue q) {
    free(q.data);
}
