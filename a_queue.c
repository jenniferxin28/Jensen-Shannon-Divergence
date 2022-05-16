#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "linkedlist.h"
#include "a_queue.h"

A_Queue * create_A_Queue()
{
	A_Queue * queue = (A_Queue *) malloc(sizeof (A_Queue));
	queue->head = NULL;
	queue->tail = NULL;
	pthread_mutex_init(&queue->lock, NULL);
	return queue;
}

int enqueue_A_Queue(A_Queue *queue, wordList * file1, wordList *file2)
{
	pthread_mutex_lock(&queue->lock);

	AQ_Node * node = (AQ_Node *) malloc(sizeof(AQ_Node));
	node->file1 = file1;
	node->file2 = file2;
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

void freeAQ_Node(AQ_Node *node)
{
	if (node)
	{
		free(node);
	}
}

int dequeue_A_Queue(A_Queue *queue, wordList ** pfile1, wordList **pfile2)
{
	int q_empty = 0;

	pthread_mutex_lock(&queue->lock);

	if (queue->head) {
		*pfile1 = queue->head->file1;
		*pfile2 = queue->head->file2;
		AQ_Node * tmp = queue->head->next;
		freeAQ_Node(queue->head);
		queue->head = tmp;
	}
	else {
		q_empty = 1;
	}

	pthread_mutex_unlock(&queue->lock);
	return q_empty;
}

void freeAQueue(A_Queue *queue)
{	
	if (queue)
	{
		AQ_Node *node = queue->head;
		while(node) {
			AQ_Node *ptr = node->next;
			freeAQ_Node(node);
			node = ptr;
		}
		free(queue);
	}
}
