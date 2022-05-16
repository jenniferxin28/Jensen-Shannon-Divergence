#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

typedef struct wordNode
{
	// Node only
	char* data; //the word
	int occur;
	float freq;
	struct wordNode* next;
} wordNode;

typedef struct wordList {
	wordNode* head;
	char filename[256];
	int total_words;
	int count;
} wordList;

wordList * create_WordList(char * filename);
void insert(wordList* list, char* word);
void printll(wordList* list);
void find_freq(wordList* list, int total);
void freeList(wordList* list);

#endif
