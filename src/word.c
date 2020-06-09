#include <math.h>
#include <string.h>

#include "word.h"



/*
 *  Finds dictionary words in a string.
 *
 *  dictionary *dict	the dictionary
 *  char *str		the string to be searched
 *  int *length		array to be filled with the lengths of all matches
 *
 *  return		number of matches (entries in the integer array length)
 */
int find_wrd(dictionary *dict, char *str, int *lengths, int n) {
	int len = 0;
	int cnt = 0;
	for (char *c = str; normalize_letter(*c) != '\0'; c++) {
		int i = normalize_letter(*c) - 'a';
		if (!(dict->next[i])) {
			break;
		}
		len++;
		dict = dict->next[i];
		if (c - str > 0 && dict->end) {
			lengths[cnt] = len;
			cnt++;
		}
	}
	for (int i = cnt; i < n; i++) {
		lengths[i] = 0;
	}
	return cnt;
}



/*
 *  returns the entropy of a dictionary word
 */
double rate_wrd(long dict_words) {
	return 16;//log2(dict_words);
}

