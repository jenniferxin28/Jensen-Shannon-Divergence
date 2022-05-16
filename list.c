#include <stdlib.h>
#include <pthread.h>
#include "linkedlist.h"
#include "list.h"

List * initList()
{
	List * list = (List *) malloc(sizeof(List));
	pthread_mutex_init(&list->lock, NULL);
	list->head = NULL;
	return list;
}

void push_to_list(List * list, wordList * item)
{
	pthread_mutex_lock(&list->lock);

	ListNode * node = (ListNode *) malloc(sizeof(ListNode));
	node->item = item;
	node->next = NULL;

	if (list->head == NULL) {
		list->head = node;
	}
	else {
		ListNode *ptr = list->head;
		while (ptr->next)
		{
			ptr = ptr->next;
		}
		ptr->next = node;
	}

	pthread_mutex_unlock(&list->lock);
}

void freeTheList(List *list)
{
	ListNode * node = list->head;
	ListNode * tmp;
	while (node) {
		freeList(node->item);
		tmp = node->next;
		free(node);
		node = tmp;
	}
	free(list);
}


