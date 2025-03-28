#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_QUEUE_SIZE		100

typedef struct Node
{
	void *data;
	struct Node *next;
} Node_t;

typedef struct QueueList
{
	size_t max;

	size_t size;
	
	Node_t *head;

	Node_t *tail;
} Queue_t;

void initialize(Queue_t *q, size_t max);

bool isEmpty(Queue_t* q);

bool enqueue(Queue_t *q, const void *data, size_t size);

bool dequeue(Queue_t *q, void *data, size_t size);

size_t get_queue_size(Queue_t *q);

void peekqueue(Queue_t *q, void *data, size_t size);

void freequeue(Queue_t *q);

#endif
