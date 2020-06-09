/* pwcheck_gtk-window.c
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

#include "pwcheck_gtk-config.h"
#include "pwcheck_gtk-window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "dictionary.h"
#include "keyboard.h"
#include "sequence.h"
#include "graph.h"

// flags for character sets, SC are non-alphanumeric characters
#define FLAG_az 1
#define FLAG_AZ 2
#define FLAG_09 4
#define FLAG_SC 8

// number of non-alphanumeric characters
#define NUM_SC 32




#define MAX_INPUT 25


/*
 *  Prints out the estimated cracking time, assuming one billion attempts per
 *  second.
 */
void print_rating(double entropy, int len) {
  return;
	printf("<p>You password entropy: %.1f bits</p>\n", entropy);
	printf("<p>Maximum entropy for this length: %.1f</p>\n", len * 6.5);
	printf("<p>Estimated cracking time: ");
	double seconds = pow(2, entropy) / 1000 / 1000 / 1000;
	if (seconds < 0.000001) {
		printf("%.0f ns</p>\n",
				seconds * 1000 * 1000 * 1000);
	} else 	if (seconds < 0.001) {
		printf("%.0f µs</p>\n",
				seconds * 1000 * 1000);
	} else if (seconds < 1.0) {
		printf("%.0f ms</p>\n",
				seconds * 1000);
	} else if (seconds < 60) {
		printf("%.0f s</p>\n",
				seconds);
	} else if (seconds < 3600) {
		printf("%.0f min</p>\n",
				round(seconds / 60));
	} else if (seconds < 3600 * 24) {
		printf("%.0f h</p>\n",
				round(seconds / 3600));
	} else if (seconds < 3600 * 24 * 365.25) {
		printf("%.0f days</p>\n",
				round(seconds / 3600 / 24));
	} else if (seconds < 3600 * 24 * 365.25 * 1000 * 1000 * 1000) {
		printf("%.0f years</p>\n",
				round(seconds / 3600 / 24 / 365.25));
	} else {
		printf("%.3e years</p>\n",
				round(seconds / 3600 / 24 / 365.25));
	}
}


/*
 *  Returns true, iff a character is a non-blank ASCII character.
 */
int printable_ascii(char c) {
	if (!('!' <= c && c <= '~')) {
		return 0;
	}
	return 1;
}



/*
 *  Computes the charset of a string.
 */
int compute_charset(char *str) {
	char flags = 0;
	for (char *c = str; *c != '\0'; c++) {
		if ('a' <= *c && *c <= 'z') {
			flags |= FLAG_az;
		} else if ('A' <= *c && *c <='Z') {
			flags |= FLAG_AZ;
		} else if ('0' <= *c && *c <= '9') {
			flags |= FLAG_09;
		} else {
			flags |= FLAG_SC;
		}
	}

	int charset = 0;
	if (flags & FLAG_az) {
		charset += 26;
	}
	if (flags & FLAG_AZ) {
		charset += 26;
	}
	if (flags & FLAG_09) {
		charset += 10;
	}
	if (flags & FLAG_SC) {
		charset += NUM_SC;
	}

	return charset;
}



/*
 *  Prints the result line for a substring starting at u, ending at v
 *
 *  char *str	the whole password string for output purposes
 *  int u	the node that starts the substring
 *  int v	the node that ends the substring
 *  graph *G	the whole graph
 *  int factor	the number of random characters are following in a row
 */
void print_table_line(const char *str, int u, int v, graph *G, int factor, GtkListStore *ls) {
	char *cat, *formula;
	int len = v - u;
	double entropy = G->edge[u][v];
	switch (G->cat[u][v]) {
		case WRD:
			cat = "dictionary";
			formula = "log₂(𝑑)";
			break;
		case SEQ:
			cat = "sequence";
			formula = "log₂(𝑘) + (𝑛 - 1) log₂(3)";
			break;
		case KBP:
			cat = "keyboard";
			formula = "log₂(𝑘) + (𝑛 - 1) log₂(9)";
			break;
		case REP:
			cat = "repetition";
			formula = "log₂(1 + 𝑖 (𝑖 + 1) / 2)";
			break;
		case RND:
			cat = "random";
			formula = "𝑛 log₂(𝑘)";
			len = factor;
			entropy *= len;
			break;
		default:
			cat = "non";
			formula = "ERROR";
			break;
	}

	char substring[len + 1];
	strncpy(substring, str + u, len);
	substring[len] = '\0';

  GtkTreeIter iter;
  gtk_list_store_append(ls, &iter);
  gtk_list_store_set(ls, &iter,
                     0, substring,
                     1, cat,
                     2, len,
                     3, formula,
                     4, entropy,
                     -1);
}



/*
 *  Computes and returns the entropy of the whole password.
 */
double compute_entropy(char *word, dictionary *dict, long dict_words, GtkListStore *ls, GtkImage *gi) {
	int n = strlen(word);
	int charset = compute_charset(word);
	graph *G = graph_new(n + 1, 6.5); //logx2(charset));
	long rep_nodes = 0;
	long rep_size = 0;
	dictionary *repetitions = dictionary_new_node(&rep_nodes);

	for (char *c = word; *c != '\0'; c++) {
		int seqlen = find_seq(c);
		int kbplen = find_kbp(c);
		int wrdlen[n];
		int wrdcnt = find_wrd(dict, c, wrdlen, n);
		int replen[n];
		int repcnt = find_wrd(repetitions, c, replen, n);

		for (int i = 3; i <= seqlen; i++) {
			graph_update_edge(G, c - word, c - word + i, rate_seq(*c, i), SEQ);
		}
		for (int i = 3; i <= kbplen; i++) {
			graph_update_edge(G, c - word, c - word + i, rate_kbp(i), KBP);
		}
		for (int i = 0; i < wrdcnt; i++) {
			graph_update_edge(G, c - word, c - word + wrdlen[i], rate_wrd(dict_words), WRD);
		}
		for (int i = 0; i < repcnt; i++) {
			graph_update_edge(G, c - word, c - word + replen[i], rate_wrd(rep_size + 1), REP);
		}

		for (int i = 0; i < c - word; i++) {
			char rep[c - word - i + 1];
			strncpy(rep, word + i, c - word - i + 1);
			rep[c - word - i + 1] = '\0';
			dictionary_add(repetitions, rep, &rep_size, &rep_nodes);
		}
	}

	int path[G->n];
	double entropy = graph_compute_path(G, path);

  gtk_list_store_clear(ls);

	int i;
	for (i = G->n - 1; path[i] == -1; i--);
	for (; i > 0; i--) {
		int j;
		for (j = i; j > 0 && path[j - 1] == path[j] + 1; j--);
		print_table_line(word, path[i], path[i - 1], G, i - j, ls);
		if (i != j) {
			i = j + 1;
		}
	}


	print_rating(entropy, n);


	graphviz(G, word, path);
  gtk_image_set_from_file(gi, "/home/stephan/.pwgraph.svg");

	graph_free(G);
	dictionary_free(repetitions);
	return entropy;
}




















struct _PwcheckGtkWindow
{
  GtkApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkButton           *bn_compute;
  GtkEntry            *te_passwd;
  GtkListStore        *ls_decomp;
  GtkTreeView         *tv_decomp;
  GtkImage            *im_graph;
};

G_DEFINE_TYPE (PwcheckGtkWindow, pwcheck_gtk_window, GTK_TYPE_APPLICATION_WINDOW)



static void
bn_compute_clicked(GtkButton        *button,
                   PwcheckGtkWindow *self)
{
  g_assert (GTK_IS_BUTTON (button));
	long dict_words = 0;
	long dict_nodes = 0;
	dictionary *dict = dictionary_new(&dict_words, &dict_nodes);
  gchar buf[30];
  strcpy(buf, gtk_entry_get_text(self->te_passwd));
  compute_entropy(buf, dict, dict_words, self->ls_decomp, self->im_graph);
}



static void
pwcheck_gtk_window_class_init (PwcheckGtkWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class, "/com/verbuech/pwcheck-gtk/pwcheck_gtk-window.ui");

  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, bn_compute);
  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, te_passwd);
  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, ls_decomp);
  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, tv_decomp);
  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, im_graph);

  gtk_widget_class_bind_template_callback (widget_class, bn_compute_clicked);
}

static void
pwcheck_gtk_window_init (PwcheckGtkWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

