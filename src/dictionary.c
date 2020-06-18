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
#include <string.h>
#include <math.h>



typedef struct dict_node {
  struct dict_node *next[26];
  int               end;
} dict_node;



struct _dictionary {
  dict_node *root;
  int        size;
};



/**
 * normalize_letter:
 * @c: the letter
 *
 * Normalizes a letter (including 1337) to lower case.
 */
static char
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



static dict_node *
dict_new_node()
{
  dict_node *dict = malloc(sizeof(dict_node));
  for (int i = 0 ; i < 26; i++) {
    dict->next[i] = NULL;
  }
  dict->end = 0;
  return dict;
}



dictionary *
dict_new()
{
  dictionary *dict = malloc(sizeof(dictionary));
  dict->root = dict_new_node();
  dict->size = 0;
  return dict;
}


dictionary *
dict_new_from_file(FILE *fd)
{
  dictionary *dict = dict_new();
  char buf[100];
  while (fgets(buf, 100, fd)) {
    dict_add_word(dict, buf);
  }
  return dict;
}



static void
dict_free_node(dict_node *dict)
{
  for (int i = 0 ; i < 26; i++) {
    if (dict->next[i] != NULL) {
      dict_free_node(dict->next[i]);
    }
  }
  free(dict);
}



void
dict_free(dictionary *dict)
{
  if (dict)
    dict_free_node(dict->root);
  free(dict);
}



void
dict_add_word(dictionary *dict,
              char       *word)
{
  if (dict && dict->root) {
    dict->size++;
    dict_node *node = dict->root;
    for (const char *c = word; normalize_letter(*c) != '\0'; c++) {
      int i = normalize_letter(*c) - 'a';
      if (node->next[i] == NULL) {
        node->next[i] = dict_new_node();
      }
      node = node->next[i];
    }
    node->end = 1;
  }
}



int
dict_find_wrd(dictionary *dict,
              char       *word,
              int        *matches)
{
  if (dict && dict->root) {
    dict_node *node = dict->root;
    int len = 0;
    int cnt = 0;
    int n = strlen(word);
    for (char *c = word; normalize_letter(*c) != '\0'; c++) {
      int i = normalize_letter(*c) - 'a';
      if (!(node->next[i])) {
        break;
      }
      len++;
      node = node->next[i];
      if (c - word > 0 && node->end) {
        matches[cnt] = len;
        cnt++;
      }
    }
    for (int i = cnt; i < n; i++) {
      matches[i] = 0;
    }
    return cnt;
  }
  return 0;
}



double
dict_rate_wrd(dictionary *dict)
{
  return log2(dict->size);
}



