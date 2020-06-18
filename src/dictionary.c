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

#include "dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>



dictionary *
dict_new()
{
  dictionary *dict = malloc(sizeof(dictionary));
  for (int i = 0 ; i < 26; i++) {
    dict->next[i] = NULL;
  }
  dict->end = 0;
  return dict;
}



dictionary *
dict_new_from_file(FILE *fd,
                   long *dict_words)
{
  dictionary *dict = dict_new();
  char buf[100];
  while (fgets(buf, 100, fd)) {
    dict_add_word(dict, buf, dict_words);
  }
  return dict;
}



void
dict_free(dictionary *dict)
{
  for (int i = 0 ; i < 26; i++) {
    if (dict->next[i] != NULL) {
      dict_free(dict->next[i]);
    }
  }
  free(dict);
}



void
dict_add_word(dictionary *dict,
              char       *word,
              long       *dict_words)
{
  (*dict_words)++;
  for (const char *c = word; normalize_letter(*c) != '\0'; c++) {
    int i = normalize_letter(*c) - 'a';
    if (dict->next[i] == NULL) {
      dict->next[i] = dict_new();
    }
    dict = dict->next[i];
  }
  dict->end = 1;
}



int
dict_find_wrd(dictionary *dict,
              char       *str,
              int        *lengths,
              int         n)
{
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



double
dict_rate_wrd(long dict_words)
{
  return log2(dict_words);
}



char
normalize_letter(char c)
{
  if ('a' <= c && c <= 'z') {
    return c;
  } else if ('A' <= c && c <= 'Z') {
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



