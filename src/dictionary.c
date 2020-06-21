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

#include <math.h>



typedef struct dict_node {
  struct dict_node *next[26];
  gboolean          end;
} dict_node;



struct _dictionary {
  dict_node *root;
  gint       size;
};



/**
 * normalize_letter:
 * @c: the letter
 *
 * Normalizes a letter (including 1337) to lower case.
 */
static gchar
normalize_letter(gchar c)
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
  dict_node *dict = g_malloc(sizeof(dict_node));
  for (gint i = 0 ; i < 26; i++) {
    dict->next[i] = NULL;
  }
  dict->end = FALSE;
  return dict;
}



dictionary *
dict_new()
{
  dictionary *dict = g_malloc(sizeof(dictionary));
  dict->root = dict_new_node();
  dict->size = 0;
  return dict;
}



dictionary *
dict_new_from_stream(GInputStream *stream)
{
  dictionary *dict = dict_new();
  GDataInputStream *dstream = g_data_input_stream_new(stream);
  gchar *l;
  while ((l = g_data_input_stream_read_line_utf8(dstream, NULL, NULL, NULL)) != NULL) {
    dict_add_word(dict, l);
  }
  g_object_unref(dstream);
  return dict;
}

static void
dict_free_node(dict_node *dict)
{
  for (gint i = 0 ; i < 26; i++) {
    if (dict->next[i] != NULL) {
      dict_free_node(dict->next[i]);
    }
  }
  g_free(dict);
}



void
dict_free(dictionary *dict)
{
  if (dict)
    dict_free_node(dict->root);
  g_free(dict);
}



void
dict_add_word(dictionary  *dict,
              const gchar *word)
{
  if (dict && dict->root) {
    dict->size++;
    dict_node *node = dict->root;
    for (const gchar *c = word; normalize_letter(*c) != '\0'; c++) {
      gint i = normalize_letter(*c) - 'a';
      if (node->next[i] == NULL) {
        node->next[i] = dict_new_node();
      }
      node = node->next[i];
    }
    node->end = TRUE;
  }
}



gsize
dict_find_wrd(dictionary  *dict,
              const gchar *word,
              gsize       *matches)
{
  if (dict && dict->root) {
    dict_node *node = dict->root;
    gsize len = 0;
    gsize cnt = 0;
    gsize n = strlen(word);
    for (const gchar *c = word; normalize_letter(*c) != '\0'; c++) {
      gint i = normalize_letter(*c) - 'a';
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
    for (gsize i = cnt; i < n; i++) {
      matches[i] = 0;
    }
    return cnt;
  }
  return 0;
}



gdouble
dict_rate_wrd(dictionary *dict)
{
  return log2(dict->size);
}



