#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "linkedlist.h"
#include "queue.h"
#include "list.h"
#include "a_queue.h"
#include "jsd.h"

mList * create_M_List()
{
	mList * list = (mList *) malloc(sizeof(mList));
	list->head = NULL;
	pthread_mutex_init(&list->lock, NULL);
	return list;
}

result_list * create_R_List()
{
	result_list * rlist = (result_list *) malloc(sizeof(result_list));	
	rlist->head = NULL;
	pthread_mutex_init(&rlist->lock, NULL);
	return rlist;
}

void free_meanNode (meanNode *node)
{
	if (node)
	{
		free(node);
	}
}

void free_mList (mList *list)
{
	if (list)
	{
		meanNode *node = list->head;
		while (node)
		{
			meanNode *ptr = node->next;
			free_meanNode (node);
			node = ptr;
		}
		free(list);
	}
}

void free_resultNode (result_node *node)
{
	if (node)
	{
		free(node);
	}
}

void free_rList (result_list *list)
{
	if (list)
	{
		result_node *node = list->head;
		while (node)
		{
			result_node *ptr = node->next;
			free_resultNode(node);
			node = ptr;
		}
		free(list);
	}
}

void insert_m_node(mList* list, char* word, double mean)
{
	pthread_mutex_lock(&list->lock);	

	meanNode* m_node = (meanNode*) malloc(sizeof(meanNode));
	m_node->mean = mean;
	m_node->word = word;
	m_node->next = NULL;

	// list is empty
	if (list->head == NULL) {
		list->head = m_node;
		pthread_mutex_unlock(&list->lock);
		return;
	}

	// list not empty, add to the end of the list
	meanNode* ptr = list->head;
	while (ptr->next) {
		ptr = ptr->next;
	}
	ptr->next = m_node;

	pthread_mutex_unlock(&list->lock);
	return;
	
}

void insert_R_List(result_list *rlist, char * f1, char *f2, int combine_words, double distance)
{
	pthread_mutex_lock(&rlist->lock);

	result_node * node = (result_node *) malloc(sizeof(result_node));
	node->file1 = f1;
	node->file2 = f2;
	node->combine_words = combine_words;
	node->distance = distance;
	node->next = NULL;

	// list is empty
	if (rlist->head == NULL) {
		rlist->head = node;
		pthread_mutex_unlock(&rlist->lock);
		return;
	}

	// list not empty, add to the end of the list
	result_node* ptr = rlist->head;
	while (ptr->next) {
		ptr = ptr->next;
	}
	ptr->next = node;

	pthread_mutex_unlock(&rlist->lock);
	return;
}	


double jsd(wordList* F1, wordList* F2)
{
	double f1_KLD = 0.0;
	double f2_KLD = 0.0;
	//linked list that holds mean and word
	mList* mList = create_M_List();
	wordNode* ptr1 = F1->head;
	wordNode* ptr2 = F2->head;
	double mean_num;

	while (ptr1 && ptr2)
	{
		//case where both are not NULL
		int search = strcmp(ptr1->data, ptr2->data);
		if (search == 0) {
			mean_num = (double)(ptr1->freq + ptr2->freq)/2.0;
			insert_m_node(mList, ptr1->data, mean_num); 
			ptr1 = ptr1->next;
			ptr2 = ptr2->next;
		} else if (search < 0) {
			// F1 word is not in F2
			// F1 = "aaa" is alphabetically above F2 = "bbb"
			mean_num = (double)(ptr1->freq)/2.0;
			insert_m_node(mList, ptr1->data, mean_num);
			ptr1 = ptr1->next;
		} else if (search > 0) {
			//F1 is below F2
			mean_num = (double)(ptr2->freq)/2.0;
			insert_m_node(mList, ptr2->data,mean_num); 
			ptr2 = ptr2->next;
		}
	}
	while (ptr1 && ptr2 == NULL)
	{
		mean_num = (double)(ptr1->freq)/2.0;
		insert_m_node(mList, ptr1->data, mean_num);
		ptr1 = ptr1->next;
	}
	while (ptr2 && ptr1 == NULL)
	{
		mean_num = (double)(ptr2->freq)/2.0;
		insert_m_node(mList, ptr2->data, mean_num);
		ptr2 = ptr2->next;
	}

	// finished with mean, now need to calculate KLD
	ptr1 = F1->head;
	ptr2 = F2->head;
	//FIXME initialize ptr3 here
	while(ptr1)
	{
		meanNode* ptr3 = mList->head;
		while (ptr3) {
			if(strcmp(ptr1->data,ptr3->word) == 0) {
				f1_KLD += ((double)ptr1->freq * log2((double)ptr1->freq / ptr3->mean));
				break;
			}
			ptr3 = ptr3->next;
		}
		ptr1 = ptr1->next;
	}

	while(ptr2)
	{
		meanNode * ptr3 = mList->head;
		while (ptr3) {
			if(strcmp(ptr2->data,ptr3->word) == 0) {
				f2_KLD += ((double)ptr2->freq * log2((double)ptr2->freq / ptr3->mean));
				break;
			} 
			ptr3 = ptr3->next;
		}
		ptr2 = ptr2->next;
	}

	double result = sqrt(f1_KLD/2.0 + f2_KLD/2.0);
	free_mList(mList);
	return result;
			
}

void printResult(result_list* rlist) 
{
	result_node *ptr = rlist->head;
	while (ptr)
	{
		printf("%f %s %s\n", ptr->distance, ptr->file1, ptr->file2);
		ptr = ptr->next;
	}
}

int rListCount(result_list * list)
{
	result_node * node = list->head;
	int count = 0;
	while (node) {
		node = node->next;
		count ++;
	}
	return count;
}

static int sortCompare(const void *p1, const void *p2)
{
	SortArray a1 = *((SortArray *) p1);
	SortArray a2 = *((SortArray *) p2);
	return a2.combine_words - a1.combine_words;
}

SortArray * sortRList (result_list * list, int * count)
{
	*count = rListCount(list);
	SortArray * sArray = (SortArray *) malloc (*count * sizeof(SortArray));
	int i = 0;
	result_node * node = list->head;
	while (node) {
		sArray[i].combine_words = node->combine_words;
		sArray[i].node = node;
		i ++;
		node = node->next;
	}

	qsort (sArray, (size_t) *count, sizeof(SortArray), sortCompare);
	return sArray;
}

void printArray(SortArray * array, int count)
{
	for (int i = 0; i < count; i ++) {
		result_node * ptr = array[i].node;
		printf("%f %s %s\n", ptr->distance, ptr->file1, ptr->file2);
	}
}
		
