#ifndef _A_QUEUE_H
#define _A_QUEUE_H

typedef struct AQ_Node {
	wordList * file1;
	wordList * file2;
	struct AQ_Node * next;
} AQ_Node;

typedef struct _A_Queue {
	AQ_Node * head;
	AQ_Node * tail;
	pthread_mutex_t lock;
} A_Queue;

A_Queue* create_A_Queue();
int enqueue_A_Queue(A_Queue *queue, wordList * file1, wordList *file2);
int dequeue_A_Queue(A_Queue *queue, wordList ** pfile1, wordList **pfile2);
void freeAQueue(A_Queue *queue);

#endif
