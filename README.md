# Jensen-Shannon-Divergence
Goal: calculate the Jensen-Shannon Divergence between two files using pthreading.
Brief explanation of all of the source files:

1. compare.c contains the main function, the consumer functions, and file/directory functions.
2. queue.c, queue.h contains the unbounded queues used by both the files and directories.
3. linkedlist.c, linkedlist.h contains the structures that hold the WFD calculations.
4. list.c, list.h holds the structs that hold the final WFD repo
5. wordfunc.c, wordfunc.h contains the functions that help calculate the WFD.
6. a_queue.c, a_queue.h contains the job queue that the analysis threads use. The job queue was used to divide up the work: while creating the queue it first splits up the files to be compared and the analysis threads would dequeue from the queue to compute the JSD.
7. jsd.c, jsd.h contains all of the functions that help compute the JSD, sort the results and print the results. 
