/* utils.h
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

#include <gtk/gtk.h>



/**
 * find_seq:
 * @str: the string
 *
 * Returns the length of the longest sequence (“abc”, “321” or “aaa”) in a string.
 */
gsize find_seq(const gchar *str);



/**
 * rate_seq:
 * @c:   first character
 * @len:  length of the sequence
 *
 * Returns the entropy of a sequence string.
 */
gdouble rate_seq(const gchar c, gsize len);



/**
 * find_kbp:
 * @str: the string
 *
 * Returns how many of the first characters of a string resemble a keyboard pattern.
 */
gsize find_kbp(const gchar *str);



/**
 * rate_kbp:
 * @len: length of the keyboard pattern
 *
 * Returns the entropy of a keyboard pattern for a length.
 */
gdouble rate_kbp(gsize len);



/**
 * compute_charset:
 * @str: the string
 *
 * Computes the charset of a string.
 */
int compute_charset(const gchar *str);



