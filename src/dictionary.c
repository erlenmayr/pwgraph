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

