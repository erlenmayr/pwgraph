#include <math.h>

#include "sequence.h"



/*
 *  Returns the length of the longest sequence (“abc”, “321” or “aaa”) in a
 *  string.
 */
int find_seq(char *str) {
	int len = 1;
	for (char *c = str; *c != '\0'; c++) {
		char x = ('A' <= c[0] && c[0] <= 'Z') ? c[0] - ('A' - 'a') : c[0];
		char y = ('A' <= c[1] && c[1] <= 'Z') ? c[1] - ('A' - 'a') : c[1];
		if (x - y == 0	|| x - y == 1 || x - y == -1) {
			len++;
		} else {
			break;
		}
	}

	return len;
}



/*
 *  Returns the entropy of a sequence string.
 *
 *  char c	first character
 *  int len	length of the sequence
 */
double rate_seq(char c, int len) {
	if (('a' <= c && c <= 'z')
			|| ('A' <= c && c <= 'Z')
			|| ('0' <= c && c <= '9')) {
		return 5 /*logx2(36)*/ + (len - 1) * log2(3);
	}
	return 0;
}

