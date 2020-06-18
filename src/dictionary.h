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

#include <stdio.h>



/*
 *  Represents a dictionary.
 *  Implemented as a tree of prefixes for efficient matching.
 */
typedef struct dictionary {
  struct dictionary *next[26];
  int end;
} dictionary;


/*
 * TODO: docs
 */
dictionary *dict_new();



/*
 *  Initializes and returns a new dictionary based on the file
 *  represented by *fd.
 *
 *  int *dict_words:  word counter for dictionary statistics
 */
dictionary *dict_new_from_file(FILE *fd, long *dict_words);



/*
 *  Recursively frees the memory of a dictionary.
 */
void dict_free(dictionary *dict);



/*
 *  Adds a single word to the dictionary. Stops at non-letter characters.
 * 
 *  int *dict_words:  word counter for dictionary statistics
 */
void dict_add_word(dictionary *dict, char *word, long *dict_words);



/*
 *  Finds dictionary words in a string.
 *
 *  dictionary *dict:  the dictionary
 *  char *str:         the string to be searched
 *  int *length:       array to be filled with the lengths of all matches
 *
 *  return:            number of matches (entries in the integer array length)
 */
int dict_find_wrd(dictionary *dict, char *str, int *lengths, int n);



/*
 *  returns the entropy of a dictionary word
 */
double dict_rate_wrd(long dict_words);



/*
 *  Normalizes a letter (including 1337) to lower case.
 */
char normalize_letter(char c);



