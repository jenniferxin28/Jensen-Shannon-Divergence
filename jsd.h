#ifndef _JSD_H
#define _JSD_H

typedef struct result_node {
	char *file1, *file2;
	int combine_words;
	double distance;
	struct result_node* next;
}result_node;

typedef struct result_list {
	result_node *head;
	pthread_mutex_t lock;
} result_list;

typedef struct SortArray {
	int combine_words;
	result_node * node;
} SortArray;


typedef struct meanNode {
	
	char* word;
	double mean;
	struct meanNode* next;
} meanNode;

typedef struct mList {
	meanNode* head;
	pthread_mutex_t lock;
} mList;


double jsd(wordList* F1, wordList* F2);
result_list * create_R_List();
void free_rList (result_list *list);
void insert_R_List(result_list *rlist, char * f1, char *f2, int combine_words, double distance);
void printResult(result_list *rlist);
SortArray * sortRList (result_list * list, int * count);
void printArray(SortArray * array, int count);

#endif
