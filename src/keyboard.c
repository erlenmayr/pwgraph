#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "keyboard.h"



// Represents a key based on its keyboard coordinates.
typedef struct {
	int row;
	int i;
} key;



// keyboard layout
const char keyboard[4][15] = {
	"`1234567890-= ",
	" qwertyuiop[]\\",
	" asdfghjkl;'  ",
	" zxcvbnm,./   "
};



/*
 *  Normalizes a character to the character that the same key gives if
 *  SHIFT key is not pressed.
 */
static char normalize_key(char c) {
	if ('a' <= c && c <= 'z') {
		return c;
	} else 	if ('A' <= c && c <= 'Z') {
		return c + ('a' - 'A');
	}
	switch (c) {
		case '`':
		case '~':
			return '`';
		case '1':
		case '!':
			return '1';
		case '2':
		case '@':
			return '2';
		case '3':
		case '#':
			return '3';
		case '4':
		case '$':
			return  '4';
		case '5':
		case '%':
			return '5';
		case '6':
		case '^':
			return '6';
		case '7':
		case '&':
			return '7';
		case '8':
		case '*':
			return '8';
		case '9':
		case '(':
			return '9';
		case '0':
		case ')':
			return '0';
		case '-':
		case '_':
			return '-';
		case '=':
		case '+':
			return '=';
		case '[':
		case '{':
			return '[';
		case ']':
		case '}':
			return ']';
		case '\\':
		case '|':
			return '\\';
		case ';':
		case ':':
			return ';';
		case '\'':
		case '"':
			return '\'';
		case ',':
		case '<':
			return ',';
		case '.':
		case '>':
			return '.';
		case '/':
		case '?':
			return '/';
	}
	return '\0';
}




/*
 *  Returns the corresponding key for an ASCII character.
 */
static key spot_key(char c) {
	key k;
	for (k.row = 0; k.row < 4; k.row++) {
		for (k.i = 0; k.i < 14; k.i++) {
			if (keyboard[k.row][k.i] == normalize_key(c)) {
				return k;
			}
		}
	}
	k.row = -1;
	k.i = -1;
	return k;
}



/*
 *  Returns 1, iff keys k and n are neighbors.
 */
static int key_neighbor(key k, key n) {
	if (abs(k.row - n.row) <= 1 && abs(k.i - n.i) <= 1) {
		return 1;
	}
	return 0;
}



/*
 *  Returns how many of the first characters of a string resemble a keyboard
 *  pattern.
 */
int find_kbp(char *str) {
	key k = spot_key(*str);
	key n;

	for (char *c = str + 1; *c != '\0'; c++) {
		n = spot_key(*c);
		if (!key_neighbor(k, n)) {
			return c - str;
		}
		k = n;
	}
	return strlen(str);
}



/*
 *  Returns the entropy of a keyboard pattern.
 */
double rate_kbp(int len) {
	double entropy = 5.5 /*logx2(47)*/ + (len - 1) * log2(9);
	return entropy;
}

