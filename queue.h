#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct q_node {
	char * data;
	struct q_node * next;
} Q_Node;

typedef struct queue {
	Q_Node * head;
	Q_Node * tail;
	pthread_mutex_t lock;
} Queue;

void init(Queue *queue);
void freeQueue(Queue *queue);
int enqueue(Queue *queue, char * data);
int dequeue(Queue *queue, char ** data);

#endif
