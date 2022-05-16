#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

void init(Queue *queue)
{
	queue->head = NULL;
	queue->tail = NULL;
	pthread_mutex_init(&queue->lock, NULL);
}

int enqueue(Queue *queue, char * data)
{
	pthread_mutex_lock(&queue->lock);

	Q_Node * node = (Q_Node *) malloc(sizeof(Q_Node));
	node->data = strdup(data);
	node->next = NULL;
	if (queue->head == NULL) {
		queue->head = node;
		queue->tail = node;
	}
	else {
		queue->tail->next = node;
		queue->tail = node;

	}
	

	pthread_mutex_unlock(&queue->lock);
	return 0;
}

void freeQNode(Q_Node *node)
{
	if (node)
	{
		free(node);
	}
}

int dequeue(Queue *queue, char ** data)
{
	int q_empty = 0;

	pthread_mutex_lock(&queue->lock);

	if (queue->head) {
		*data = queue->head->data;
		Q_Node * tmp = queue->head->next;
		freeQNode(queue->head);
		queue->head = tmp;
	}
	else {
		q_empty = 1;
	}

	pthread_mutex_unlock(&queue->lock);
	return q_empty;
}

void freeQueue(Queue *queue)
{	
	if (queue)
	{
		Q_Node *node = queue->head;
		while(node) {
			Q_Node *ptr = node->next;
			freeQNode(node);
			node = ptr;
		}
	}
}
