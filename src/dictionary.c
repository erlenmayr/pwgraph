/* dictionary.c
 *
 * Copyright 2020 Stephan Verbücheln
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "dictionary.h"



/*
 * Normalizes a letter (including 1337) to lower case.
 */
char normalize_letter(char c) {
	if ('a' <= c && c <= 'z') {
		return c;
	} else 	if ('A' <= c && c <= 'Z') {
		return c + ('a' - 'A');
	} else {
		switch (c) {
			case '4':
			case '@':
				return 'a';
			case '(':
				return 'c';
			case '3':
				return 'e';
			case '6':
				return 'g';
			case '!':
				return 'i';
			case '1':
				return 'l';
			case '0':
				return 'o';
			case '5':
				return 's';
			case '7':
				return 't';
			case '2':
				return 'z';
		}
	}
	return '\0';
}



/*
 * Allocates a new node for the dictionary.
 *
 * long *dict_nodes	node counter for dictionary statistics
 */
dictionary *dictionary_new_node(long *dict_nodes) {
	(*dict_nodes)++;
	dictionary *dict = malloc(sizeof(dictionary));
	for (int i = 0 ; i < 26; i++) {
		dict->next[i] = NULL;
	}
	dict->end = 0;
	return dict;
}



/*
 *  Recursively frees the memory of a dictionary.
 */
void dictionary_free(dictionary *dict) {
	for (int i = 0 ; i < 26; i++) {
		if (dict->next[i] != NULL) {
			dictionary_free(dict->next[i]);
		}
	}
	free(dict);
}



/*
 *  Adds a single word to the dictionary. Stops at non-letter characters.
 *
 *  int *dict_words	word counter for dictionary statistics
 *  int *dict_nodes	node counter for dictionary statistics
 */
void dictionary_add(dictionary *dict, char *word, long *dict_words, long *dict_nodes) {
	(*dict_words)++;
	for (const char *c = word; normalize_letter(*c) != '\0'; c++) {
		int i = normalize_letter(*c) - 'a';
		if (dict->next[i] == NULL) {
			dict->next[i] = dictionary_new_node(dict_nodes);
		}
		dict = dict->next[i];
	}
	dict->end = 1;
}



/*
 *  Initializes and returns a new dictionary based on the file
 *  "/usr/share/dict/words".
 *
 *  int *dict_words	word counter for dictionary statistics
 *  int *dict_nodes	node counter for dictionary statistics
 */
dictionary *dictionary_new(long *dict_words, long *dict_nodes) {
	dictionary *dict = dictionary_new_node(dict_nodes);
	FILE *words = fopen("/home/stephan/pwcheck/dictionary.txt", "r");
	if (!words) {
		fprintf(stderr, "FATAL ERROR: Could not open \"dictionary.txt\".\n");
		exit(-1);
	}
	char buf[100];
	while (fgets(buf, 100, words)) {
		dictionary_add(dict, buf, dict_words, dict_nodes);
	}
	fclose(words);
	return dict;
}



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
	return log2(dict_words);
}



