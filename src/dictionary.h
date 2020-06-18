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



/**
 * dictionary:
 *
 * Represents a dictionary, implemented as a tree of prefixes for efficient matching.
 */
typedef struct _dictionary dictionary;



/**
 * dict_new:
 *
 * Creates a new dictionary (i.e. tree node).
 */
dictionary *dict_new();



/**
 * dict_new_from_file:
 * @fd: a text file with one word per line
 *
 * Initializes and returns a new dictionary based on the file represented by *fd.
 */
dictionary *dict_new_from_file(FILE *fd);



/**
 * dict_free:
 * *dict: the dictionary
 *
 * Recursively frees the memory of a dictionary.
 */
void dict_free(dictionary *dict);



/**
 * dict_add_word:
 * @dict: the dictionary
 * @word: the word to add
 * 
 * Adds a single word to the dictionary. Stops at non-letter characters.
 */
void dict_add_word(dictionary *dict, char *word);



/**
 * dict_find_wrd:
 * @dict: the dictionary
 * @word: the string to be searched
 * @matches: array to be filled with the lengths of all matches
 *
 * Finds dictionary words in a string.
 *
 * Returns: number of matches (entries in the integer array length)
 *
 */
int dict_find_wrd(dictionary *dict, char *word, int *matches);



/**
 * dict_rate_wrd:
 * @dict: the dictionary
 *
 * Computes the entropy of a dictionary word.
 */
double dict_rate_wrd(dictionary *dict);



