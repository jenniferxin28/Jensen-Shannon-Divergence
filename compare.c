#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <dirent.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <ctype.h>
#include <math.h>
#include "queue.h" 
#include "linkedlist.h"
#include "wordfunc.h"
#include "list.h"
#include "a_queue.h"
#include "jsd.h"

#define DASHES printf("------------------------------------------------\n");
#define NEWLINE printf("\n");
#define HELLO printf("hello\n");


typedef struct repo_list {
	wordList * repo;
	struct repo_list * next;
	int count;
	pthread_mutex_t lock;
} repo_list;

typedef struct qargs {
	Queue *FQ;
	Queue *DQ;
	A_Queue *AQ;
	result_list *rlist;
	List * list;
    int id;
}qargs;

//global variables
int i, j;
int flag = 0;
int fQueueCount = 0;
char dirThreadVal[256];
char fileThreadVal[256];
char analThreadVal[256];
char suffix[256];
int dir_threads = 1;
int file_threads = 1;
int ana_threads = 1;
//char change_suffix[] = ".txt"; //FIXME
int dCheck = 0;
int fCheck = 0;
int aCheck = 0;
int sCheck = 0;
int optCheck = 0;



//if a begins with b
int beginsWith(const char *a, const char *b){

   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

//if a ends with b
int endsWith(const char *a, const char *b){

	int wordLen = strlen(a);
	int sufLen = strlen(b);

	return (wordLen >= sufLen) && (0 == strcmp(a + (wordLen-sufLen), b));
}

// directory = 1
// file = 0
int isdir(char *name) {

	struct stat data;
	int err = stat(name, &data);
	
	// should confirm err == 0
	if (err) {
		perror(name);  // print error message
		return 0;
	}
	
	if (S_ISDIR(data.st_mode)) {
		// S_ISDIR macro is true if the st_mode says the file is a directory
		// S_ISREG macro is true if the st_mode says the file is a regular file
		return 1;
	} 
	
	return 0;
}	

void printQueue(Queue *Q){
	Q_Node *node = Q->head;
	while(node != NULL)
	{
		printf("Node data is: %s\n", node->data);
		node = node->next;
	}

}

// file consumer, should run wfd
void* fconsumer(void* A) {
	
	qargs *args = (qargs *)A;
	char* j; 

	while (1) {
		if(dequeue(args->FQ, &j) != 0) {
			// case 1: thread has reached the end of queue
			// case 2: no work to be found
			// perror("dequeue failed");
			sleep(1);
			return NULL;
		}
		//printf("Dequeued element: %s\n", j); commented out no error
		//int fp = open(j, O_RDWR);
		int fp = open(j, O_RDONLY);
		if(fp == -1) {
			flag = 1;
			perror("Error");
			continue;
		}

		wordList * repo = create_WordList(j);
		free(j);
		wfd(fp, repo);
		push_to_list (args->list, repo);

		close(fp);
		
	}


	return NULL;
}

//directory consumer
void* dconsumer(void* A) {
	//printf("dconsumer dir_threads: %d\n", dir_threads);
	//NEWLINE
	qargs *args = (qargs *) A;
	
	while (args->DQ->head){
		
		DIR *fp_dir;
		struct dirent *sd;
		char* fn;
		char * foo;

		char dpath[1000];
		char fpath[1000];

		fp_dir = opendir(args->DQ->head->data);
		// printf("current dir: %s\n", args->DQ->head->data);

		if (fp_dir == NULL){
			perror("Error");
			exit(EXIT_FAILURE);
		}	

		while((sd = readdir(fp_dir)) != NULL){
			
			//FILE - add to fqueue 
			if (sd->d_type == 8){ //dir = 4, file = 8

				fn = sd->d_name; //bar.txt
				if (beginsWith(fn, ".")){
					continue;
				}

				strcpy(fpath, args->DQ->head->data);
				strcat(fpath, "/");
				strcat(fpath, fn);

				// -s is used, so file must end in given suffix OR
				// -s not used, so file must end in .txt
				if ((endsWith(fn, suffix) && sCheck == 1) || (endsWith(fn,".txt") && sCheck == 0)){ 
					// printf("FILE ENQUEUE: %s\n", fpath);
					fQueueCount++;
					enqueue(args->FQ, fpath);
				}
				
			}	

			//DIRECTORY - recurse and traverse
			else if (sd->d_type == 4){

				if (beginsWith(sd->d_name, ".")){
					continue;
				}	

				strcpy(dpath, args->DQ->head->data);
				strcat(dpath, "/");
				strcat(dpath, sd->d_name);


				//printf("DIR ENQUEUE: %s\n", dpath);
				enqueue(args->DQ, dpath);
			}
			
		}

		dequeue(args->DQ, &foo);
		free(foo);

		closedir(fp_dir);
	//	NEWLINE;
	}
	//DASHES
	return NULL;
}

void *aconsumer(void *A) 
{
	qargs *args = (qargs *) A;
	wordList *f1, *f2;

	while (1)
	{
		int empty = dequeue_A_Queue(args->AQ, &f1, &f2);
		if (empty)
			break;
		int combine_words = f1->total_words + f2->total_words;
		double distance = jsd(f1, f2);
		//printf("compare result : distance = %lf\n", distance);
		insert_R_List(args->rlist,f1->filename, f2->filename, combine_words, distance);
	}
	return NULL;
}

//builds analysis job queue
A_Queue * build_A_Queue(List* list)
{
	A_Queue * queue = create_A_Queue();
	ListNode * node1 = list->head;
	if (node1 == NULL) 
		return queue;

	while (node1 && node1->next) {
		ListNode * node2 = node1->next;
		while (node2) {
			enqueue_A_Queue(queue, node1->item, node2->item);
			node2 = node2->next;
		}
		node1 = node1->next;
	}
	return queue;
}

int fileExists(const char *path){
    if (access(path, F_OK) == -1){
        return 0;
	}
    return 1;
}

int main (int argc, char* argv[]) {

   
   	// both are unbounded queues
    Queue fqueue; // file queue
	init(&fqueue);

	Queue dqueue; // directory queue
	init(&dqueue);

	List * list = initList();

    //iterate through input arguments
    for (int i = 1; i < argc; i++){
        
		//optional arguments only be first four arguments
		if( (beginsWith(argv[i], "-") == 1) && ((i >= 5) || (optCheck == 1)) ){
			fprintf(stderr, "Error: Please input all optional arguments before all regular arguments and input each optional argument at most once.\n");
			exit(0);
		}
        //optional arguments
        if(beginsWith(argv[i], "-") == 1){
			
            if( beginsWith(argv[i], "-d") == 1 || beginsWith(argv[i], "-f") == 1 || beginsWith(argv[i], "-a") == 1){
    
                //-------ERROR CHECKING--------
                
                //missing argument
                if (argv[i][2] == '\0'){
                    fprintf(stderr, "Error: %s is missing an argument\n", argv[i]);
                    exit(0);
                }

                //invalid argument
                if (argv[i][2] == '0'){
                    fprintf(stderr, "Error: %s is an invalid argument\n", argv[i]);
					//return EXIT_FAILURE;
					exit(0);
                }

                for(j = 2; j < strlen(argv[i]); j++){
                    if(isalpha(argv[i][j]) || !isdigit(argv[i][j])){
                        fprintf(stderr, "Error: %s is an invalid argument\n", argv[i]);
						exit(0);
                    }
                }
                //---------------------------------------------------------

                if (beginsWith(argv[i], "-d") == 1){
					
					if (dCheck == 0){
						dCheck = 1;

						for(j = 0; j < strlen(argv[i]); j++){	
							dirThreadVal[j] = argv[i][j + 2];					
						}

						dir_threads = atoi(dirThreadVal);	
						//printf("dir_threads: %d\n", dir_threads);
					}
					else{
						fprintf(stderr, "Error: The -d argument must occur at most once\n");
						exit(0);
					}						
                }
                
                if (beginsWith(argv[i], "-f") == 1){
                    
					if(fCheck == 0){
						fCheck = 1;

						for(j = 0; j < strlen(argv[i]); j++){
							fileThreadVal[j] = argv[i][j + 2];
						}

						file_threads = atoi(fileThreadVal);
						//printf("file_threads: %d\n", file_threads);
					}
					else{
						fprintf(stderr, "Error: The -f argument must occur at most once\n");
						exit(0);
					}                    
                }

                if (beginsWith(argv[i], "-a") == 1){

					if(aCheck == 0){
						aCheck = 1;

						for(j = 0; j < strlen(argv[i]); j++){						
							analThreadVal[j] = argv[i][j + 2];
						}
						ana_threads = atoi(analThreadVal);
						//printf( "ana threads: %d\n", ana_threads);
					}
					else{
						fprintf(stderr, "Error: The -a argument must occur at most once\n");
						exit(0);
					}                    
                }
                
            }

            else if (beginsWith(argv[i], "-s") == 1){
				
				if(sCheck == 0){
					//file name suffix 
					sCheck = 1; //-s is inputted
					for(j = 0; j < strlen(argv[i]); j++){
						suffix[j] = argv[i][j + 2];
					}
				}
				else{
					fprintf(stderr, "Error: The -s argument must occur at most once\n");
					exit(0);
				}
            }

            //invalid option
            else {
                fprintf(stderr, "Error: %s is an invalid option\n", argv[i]);
                exit(0);
            }
        }
              
        //input is file so add to file queue and doesnt need to end in suffix
        else if(isdir(argv[i]) == 0){

			if(fileExists(argv[i]) == 1){
				optCheck = 1;
				fQueueCount++;
				enqueue(&fqueue, argv[i]);
			}
			else{
				flag = 1;
			}
        }

        //input is directory so add to directory queue, traverse 
        else if(isdir(argv[i]) == 1){
			optCheck = 1;
            enqueue(&dqueue, argv[i]);
        }

        else {
			fprintf(stderr, "Error: Inputs are not all files, directories, or optional arguments");
			exit(0);
        }

    }
/*
	NEWLINE
    printf("DIR QUEUE [before]:\n");
    printQueue(&dqueue);
    NEWLINE;
    printf("FILE QUEUE [before]:\n");
    printQueue(&fqueue);
	NEWLINE;
	DASHES
	NEWLINE;
*/
    // collection phase, start synchronized file and directory threads to process the 2 queues
	// directory thread will repeatedly dequeue, and traverse and add to file queue
	// file thread will process and compute wcd
	qargs *dargs; // args for dir
	qargs *fargs; //args for file
	pthread_t *ftids; // file thread IDs
	pthread_t *dtids; // dir thread IDs
	
	// FIXME, add directory loop
	dargs = malloc(dir_threads * sizeof(qargs));
	dtids = malloc(dir_threads * sizeof(pthread_t));
	
	if(dqueue.head){
			
		for (int i = 0; i < dir_threads; i++) {
			dargs[i].FQ = &fqueue;
			dargs[i].DQ = &dqueue;
			dargs[i].id = i;
			pthread_create(&dtids[i], NULL, dconsumer, &dargs[i]);
			//printf("in loop dir_threads: %d\n", dir_threads);
			pthread_join(dtids[i], NULL);			
		}
		
	}
/*
	NEWLINE;
	printf("DIR QUEUE [after]:\n");
	printQueue(&dqueue);
	NEWLINE;
	printf("FILE QUEUE [after]:\n");
    printQueue(&fqueue);
    NEWLINE;
	DASHES;
*/	

	// malloc, free ltr
	fargs = malloc(file_threads * sizeof(qargs));
	ftids = malloc(file_threads * sizeof(pthread_t));
	

	for (int i = 0; i < file_threads; i++) { 
		fargs[i].FQ = &fqueue;
		fargs[i].id = i;
		fargs[i].list = list;
		//printf("I'm in an analysis thread!\n");
		pthread_create(&ftids[i], NULL, fconsumer, (void*) &fargs[i]);
		pthread_join(ftids[i], NULL);
	}
	/*
	for (int i = 0; i < dir_threads; i++) {
		pthread_join(dtids[i], NULL);
	}
	for (int i = 0; i < file_threads; i++) {
		pthread_join(ftids[i], NULL);
	}
	*/
	// taking WFD repo and calculating the frequency
	ListNode * node = list->head;
	int count = 0;
	while (node) {
		count = node->item->total_words;
//		NEWLINE
//		printf("File Name: %s\n",node->item->filename);
//		printf("Total words: %d\n", node->item->total_words);
//		NEWLINE
//		printll(node->item);
		find_freq(node->item, count);
//		NEWLINE
//		DASHES
		node = node->next;
	}
//	NEWLINE

	//printf("file queue count: %d", fQueueCount);
	if(fQueueCount < 2){
		fprintf(stderr, "Error: There is less than two files to be analyzed.\n");
		exit(0);
	}

    A_Queue* a_queue = build_A_Queue(list);

	// not enough files or something lol 
	if (a_queue == NULL) {
		return EXIT_FAILURE;
	}
	
	qargs *a_args;
	pthread_t *atids;
	a_args = malloc(ana_threads *sizeof(qargs));
	atids = malloc(ana_threads * sizeof(pthread_t));
	// initialize result_list;
	result_list *rlist = create_R_List();

	for (int i = 0; i < ana_threads; i++) {
		a_args[i].AQ = a_queue;
		a_args[i].rlist = rlist;
		pthread_create(&atids[i], NULL, aconsumer, (void *)&a_args[i]);
	}
	for (int i = 0; i < ana_threads; i++) {
		pthread_join(atids[i], NULL);
	}
//	NEWLINE

	int comparisons;
	SortArray * sorted = sortRList(rlist, &comparisons);

	printArray(sorted, comparisons);
	// printResult(rlist);
	/*
	printf("dir: %s\n", dirThreadVal);
    printf("file: %s\n", fileThreadVal);
    printf("anal: %s\n", analThreadVal);
    printf("suffix: %s\n", suffix);
	*/
	
    //FREEDOM
	freeQueue(&fqueue);
	freeQueue(&dqueue);

	free(sorted);
	freeTheList(list);

	free(ftids);
    free(fargs);
	free(dtids);
	free(dargs);
	free(atids);
	free(a_args);
	freeAQueue(a_queue);
	free_rList(rlist);

	if (flag == 1){
        //printf("FAIL\n");
        return EXIT_FAILURE;
    }
    else {
        //printf("SUCCESS\n");
        return EXIT_SUCCESS;
    }
    return 0;
}
