#ifndef _LIST_H
#define _LIST_H

typedef struct listNode
{
	wordList * item;
	struct listNode * next;
} ListNode;

typedef struct list
{
	ListNode * head;
	pthread_mutex_t lock;
} List;

List * initList();
void push_to_list(List * list, wordList * item);
void freeTheList(List * list);

#endif
