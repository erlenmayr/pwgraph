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
dictionary *dictionary_new(long *dict_words, long *dict_nodes);




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

