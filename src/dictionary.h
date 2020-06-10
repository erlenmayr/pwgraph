/* dictionary.h
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

// represents a dictionary
typedef struct dictionary {
	struct dictionary *next[26];
	int end;
} dictionary;



/*
 * Normalizes a letter (including 1337) to lower case.
 */
char normalize_letter(char c);



/*
 *  Recursively frees the memory of a dictionary.
 */
void dictionary_free(dictionary *dict);



/*
 *  Adds a single word to the dictionary. Stops at non-letter characters.
 * 
 *  int *dict_words	word counter for dictionary statistics
 *  int *dict_nodes	node counter for dictionary statistics
 */
void dictionary_add(dictionary *dict, char *word, long *dict_words, long *dict_nodes);



/*
 *  Initializes and returns a new dictionary based on the file
 *  "/usr/share/dict/words".
 * 
 *  int *dict_words	word counter for dictionary statistics
 *  int *dict_nodes	node counter for dictionary statistics
 */
dictionary *dictionary_new(long *dict_words, long *dict_nodes, FILE *fd);




dictionary *dictionary_new_node(long *dict_nodes);




/*
 *  Finds dictionary words in a string.
 *
 *  dictionary *dict	the dictionary
 *  char *str		the string to be searched
 *  int *length		array to be filled with the lengths of all matches
 *
 *  return		number of matches (entries in the integer array length)
 */
int find_wrd(dictionary *dict, char *str, int *lengths, int n);



/*
 *  returns the entropy of a dictionary word
 */
double rate_wrd(long dict_words);



