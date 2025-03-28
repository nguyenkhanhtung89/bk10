
#include "queue.h"


void initialize(Queue_t *q, size_t max)
{
	q->max = max;
	q->size = 0;
	q->head = q->tail = NULL;
}

bool isEmpty(Queue_t* q)
{
	if (q->head == NULL)
	{
		return true;
	}
	return false;
}

bool enqueue(Queue_t *q, const void *data, size_t size)
{
	/* Queue full */
	if (q->size >= q->max)
	{
		return false;
	}

	Node_t *newNode = (Node_t *)malloc(sizeof(Node_t));

	if (newNode == NULL)
	{
		return false;
	}

	/* Set data */
	newNode->next = NULL;
	newNode->data = (void *)malloc(size);
	memcpy(newNode->data, data, size);

	if (isEmpty(q))
	{
		q->head = q->tail = newNode;
	}
	else
	{
		q->tail->next = newNode;
		q->tail = newNode;
	}

	q->size += 1;

	return true;
}

bool dequeue(Queue_t *q, void *data, size_t size)
{
	if (q->size > 0)
	{
		Node_t *temp = q->head;

		if (temp == NULL)
		{
			data = NULL;
		}
		else
		{
			memcpy(data, temp->data, size);
		}

		/* Check size */
		if (q->size > 1)
		{
			q->head = q->head->next;
		}
		else
		{
			q->head = NULL;
			q->tail = NULL;
		}

		q->size -= 1;

		if (temp->data != NULL)
		{
			free(temp->data);
		}
		if (temp != NULL)
		{
			free(temp);
		}
	}
}

size_t get_queue_size(Queue_t *q){
	if (q->head == NULL){
		return 0;
	}
	return q->size;
}

void peekqueue(Queue_t *q, void *data, size_t size){
	if (q->size > 0)
	{
		Node_t *temp = q->head;

		if (temp == NULL)
		{
			data = NULL;
		}
		else
		{
			memcpy(data, temp->data, size);
		}
	}
}

void freequeue(Queue_t *q){
	if (q->size > 0)
	{
		Node_t *temp = q->head;

		/* Check size */
		if (q->size > 1)
		{
			q->head = q->head->next;
		}
		else
		{
			q->head = NULL;
			q->tail = NULL;
		}

		q->size -= 1;

		if (temp->data != NULL)
		{
			free(temp->data);
		}
		if (temp != NULL)
		{
			free(temp);
		}
	}
}
