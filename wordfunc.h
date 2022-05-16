#ifndef _WORDFUNC_H
#define _WORDFUNC_H

void removePunct (char* str, char toRem);
char* word_fix(char* str);
int findWord_LL(wordList* head, char* word);
int findWord(char* buf, int bufIndex, int bufWidth, char* word);
void wfd ( int input, wordList* list);

#endif

