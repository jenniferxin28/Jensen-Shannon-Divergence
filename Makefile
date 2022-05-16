compare: compare.c queue.c linkedlist.c wordfunc.c list.c a_queue.c jsd.c
	gcc -g -Wvla -Wall -fsanitize=address,undefined -pthread -o compare linkedlist.c compare.c queue.c wordfunc.c list.c a_queue.c jsd.c -lm
