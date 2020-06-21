/* utils.c
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

#include "utils.h"

#include <math.h>



gsize
find_seq(const gchar *str)
{
  gsize len = 1;
  for (const char *c = str; *c != '\0'; c++) {
    gchar x = ('A' <= c[0] && c[0] <= 'Z') ? c[0] - ('A' - 'a') : c[0];
    gchar y = ('A' <= c[1] && c[1] <= 'Z') ? c[1] - ('A' - 'a') : c[1];
    if (x - y == 0 || x - y == 1 || x - y == -1) {
      len++;
    } else {
      break;
    }
  }

  return len;
}



double
rate_seq(const gchar c,
         gsize       len)
{
  if (('a' <= c && c <= 'z')
      || ('A' <= c && c <= 'Z')
      || ('0' <= c && c <= '9')) {
    return log2(36) + (len - 1) * log2(3);
  }
  return 0;
}



/*
 * Represents a key based on its keyboard coordinates.
 */
typedef struct {
  gint row;
  gint i;
} key;



/*
 * keyboard layout (US)
 */
const gchar keyboard[4][15] = {
  "`1234567890-= ",
  " qwertyuiop[]\\",
  " asdfghjkl;'  ",
  " zxcvbnm,./   "
};



/*
 *  Normalizes a character to the character that the same key gives if
 *  SHIFT key is not pressed.
 */
static gchar
normalize_key(const gchar c)
{
  if ('a' <= c && c <= 'z') {
    return c;
  } else if ('A' <= c && c <= 'Z') {
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
static key
spot_key(const gchar c)
{
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
static gboolean
key_neighbor(key k,
             key n)
{
  if (abs(k.row - n.row) <= 1
      && abs(k.i - n.i) <= 1) {
    return TRUE;
  }
  return FALSE;
}



gsize
find_kbp(const gchar *str)
{
  key k = spot_key(*str);
  key n;

  for (const gchar *c = str + 1; *c != '\0'; c++) {
    n = spot_key(*c);
    if (!key_neighbor(k, n)) {
      return c - str;
    }
    k = n;
  }
  return strlen(str);
}



gdouble
rate_kbp(gsize len)
{
  gdouble entropy = log2(47) + (len - 1) * log2(9);
  return entropy;
}



/*
 * Flags for character sets
 * az: lower-case letters
 * AZ: upper-case letters
 * 09: digits
 * SC: special characters
 */
typedef enum {
  CHARSET_NONE = 0x0,
  CHARSET_az = 0x1,
  CHARSET_AZ = 0x2,
  CHARSET_09 = 0x4,
  CHARSET_SC = 0x8
} csflags;



gint
compute_charset(const gchar *str)
{
  csflags flags = CHARSET_NONE;
  for (const gchar *c = str; *c != '\0'; c++) {
    if ('a' <= *c && *c <= 'z') {
      flags |= CHARSET_az;
    } else if ('A' <= *c && *c <='Z') {
      flags |= CHARSET_AZ;
    } else if ('0' <= *c && *c <= '9') {
      flags |= CHARSET_09;
    } else {
      flags |= CHARSET_SC;
    }
  }

  gint charset = 0;
  if (flags & CHARSET_az) {
    charset += 26;
  }
  if (flags & CHARSET_AZ) {
    charset += 26;
  }
  if (flags & CHARSET_09) {
    charset += 10;
  }
  if (flags & CHARSET_SC) {
    // number of ASCII chars which are neither letters nor digits
    charset += 32;
  }

  return charset;
}



