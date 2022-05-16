#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "linkedlist.h"
#include "wordfunc.h"

wordList * create_WordList(char * filename)
{
	wordList * list = (wordList *) malloc(sizeof(wordList));
	list->head = NULL;
	strcpy(list->filename, filename);
	list->total_words = 0;
	list->count = 0;
	return list;
}

// inserts nodes alphabetically
void insert(wordList* list, char* word)
{	
	//wordNode * head = list->head;
	if(findWord_LL(list, word) == 0) {
		wordNode* newNode = (wordNode*) malloc(sizeof(wordNode));
		newNode->occur = 1;
		newNode->data = (char*) malloc(strlen(word) + 1);
		strcpy(newNode->data, word);
		newNode->next = NULL;

		// 1. head is NULL
		if (list->head == NULL) {
			list->head = newNode;
			list->total_words = 1;
			list->count = 1;
			return;
		}

		// 2. insert at head
		// greater than head, becomes new head
		if (strcmp(newNode->data, list->head->data) < 0) {
			newNode->next = list->head;
			list->head = newNode;
			list->total_words++;
			list->count++;
			return;
		} 

		// 3. middle of the list
		wordNode* curr = list->head->next;
		wordNode* prev = list->head;
		while (curr)  {
			
			if (strcmp(newNode->data, curr->data) < 0) {
				prev->next = newNode;
				newNode->next = curr;
				list->total_words++;
				list->count++;
				return;
			}	
			curr = curr->next;
			prev = prev->next;
		}

		// 4. add at the end
		prev->next = newNode;
		list->count++;
		list->total_words++;
		return;
	}
}

void printll(wordList* list) 
{
	wordNode* curr = list->head;
	while(curr) {
		printf("%s\n", curr->data);
		//printf("Num of occurances is %d\n", curr->occur);
		curr = curr->next;
	}
}

void find_freq(wordList* list, int total)
{
	wordNode* curr = list->head;
	while(curr) {
		curr->freq =  ((float)curr->occur /(float) total);
		//printf("Frequency : %f\n", curr->freq);
		curr = curr->next;
	}
}
	
void freeNode(wordNode * node)
{
	if (node) {
		if (node->data)
			free(node->data);
		free(node);
	}
}

void freeList(wordList* list)
{
	//printf("freeList() ...\n");
	if(list) {
		wordNode* node = list->head;
		while (node) {
			wordNode* nxt = node->next;
			freeNode(node);
			node = nxt;
		}
		free(list);
	}
}


