#include "dictionary.h"



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

