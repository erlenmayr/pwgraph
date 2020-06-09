/* patterns.c
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

#include <math.h>

#include "patterns.h"



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

