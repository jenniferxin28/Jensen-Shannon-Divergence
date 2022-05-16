#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "linkedlist.h"

void removePunct (char* str, char toRem)
{
	int old = 0;
	int new = 0;
	while (str[old]) {
		if (str[old] != toRem) {
			str[new++] = str[old];
		}
		old++;
	}
	str[new] = 0;
}

char* word_fix(char* str)
{
	for (int i = 0; i <strlen(str); i++) {
		str[i] = tolower(str[i]);
		if (ispunct(str[i])) removePunct(str, str[i]);
		if (str[i] == '\'') removePunct(str, '\'');
	}
	return str;
}

// found word, increases count, returns 1
// no word, returns 0
int findWord_LL(wordList* list, char* word)
{
	wordNode* curr = list->head;
	while (curr) {
		if(strcmp(curr->data, word) == 0) {
			curr->occur++;
			list->total_words++;
			return 1;
		}
		curr = curr->next;
	}
	return 0;
}

int findWord(char* buf, int bufIndex, int bufWidth, char* word)
{

	int i = bufIndex;
	int startIndex = -1;
	int endIndex = -1;

	while (i < bufWidth) {
		if (startIndex == -1) {
			if (!isspace(buf[i])) {
				startIndex = i;
			} 
			
		}
		else {
			if(isspace(buf[i])) {
				endIndex = i;
				break;
			}
		}
		i++;
	}
	if (startIndex == -1) {
		word[0] = '\0';
		return bufWidth;
	} else {
		if (endIndex == -1) {
			memcpy(word, buf + startIndex, bufWidth - startIndex);
			word[bufWidth - startIndex] = '\0';
			return bufWidth;
		}
		else {
			memcpy(word, buf + startIndex, endIndex - startIndex);
			word[endIndex - startIndex] = '\0';
			return endIndex;
		}
	}
}

#define bufWidth 256
void wfd ( int input, wordList* list) 
{
	char word[bufWidth + 1];
	char* buf = (char*) malloc(bufWidth * sizeof(char));
	char* stash = NULL; // stores partial words
	int stashLen= 0;
	int bufLen = 0;
	while ((bufLen = read(input, buf, bufWidth))) {
		int buf_index = 0;
		while (1) {
			int new_index;

			if(isspace(buf[0]) && buf_index == 0 && stash) {
				// 2nd line first character is space, insert stash

				insert(list, word_fix(stash));
				free(stash);
				stash = NULL;
			}
			new_index = findWord(buf, buf_index, bufLen, word);
			
			if (new_index < bufLen) {
				
				if (buf_index == 0 && stash) {
					stash = (char*) realloc(stash, stashLen + strlen(word));
					memcpy(stash + stashLen, word, strlen(word));
					insert(list, word_fix(stash));
					free(stash);
					stash = NULL;
				}
				else {
					insert(list, word_fix(word));
				} 
				buf_index = new_index;
				continue;
			}
			else {
				// stash
				if (strlen(word) > 0) {
					if (stash == NULL) {
						stashLen = strlen(word)+1;
						stash = (char *) malloc(stashLen);
						memcpy(stash, word, stashLen);
					}
					else {
						stash = (char *) realloc (stash, stashLen + strlen(word));
						memcpy(stash + stashLen, word, strlen(word));
						stashLen += strlen(word);
					}
				}
				break;
			}
		}
	}
	if (stash && stashLen > 0) {
		insert(list, word_fix(stash));
	}

	if (stash) 
		free(stash);
	if (buf)
		free(buf);

}
