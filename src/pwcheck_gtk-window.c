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
#include <sys/stat.h>

#include "dictionary.h"
#include "patterns.h"
#include "graph.h"

// flags for character sets, SC are non-alphanumeric characters
#define FLAG_az 1
#define FLAG_AZ 2
#define FLAG_09 4
#define FLAG_SC 8



/*
 *  Prints information about the password to a GtkLabel.
 */
void
print_rating(double   entropy,
             int      len,
             GtkLabel *label)
{
  char buf[128];
  sprintf(buf,
          "You password entropy: %.1f bits\nMaximum entropy for this length: %.1f bits\n",
          entropy,
          len * log2(94));
	gtk_label_set_text(label, buf);
}



/*
 *  Computes the charset of a string.
 */
int
compute_charset(char *str)
{
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
    // number of ASCII chars which are neither letters nor digits
		charset += 32;
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
void
list_store_append_substring(const char   *str,
                            int          u,
                            int          v,
                            graph        *G,
                            int          len,
                            GtkListStore *ls)
{
	char *cat, *formula;
	int pathlen = v - u;
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
      pathlen = len;
			entropy *= len;
			break;
		default:
			cat = "non";
			formula = "ERROR";
			break;
	}

	char substring[pathlen + 1];
	strncpy(substring, str + u, pathlen);
	substring[pathlen] = '\0';

  GtkTreeIter iter;
  gtk_list_store_append(ls, &iter);
  gtk_list_store_set(ls, &iter,
                     0, substring,
                     1, cat,
                     2, pathlen,
                     3, formula,
                     4, entropy,
                     -1);
}



/*
 *  Computes and returns the entropy of the whole password.
 */
double
compute_entropy(char          *word,
                dictionary    *dict,
                long          dict_words,
                GtkListStore  *ls,
                GtkImage      *gi,
                GtkLabel      *label)
{
	int n = strlen(word);
	int charset = compute_charset(word);
	graph *G = graph_new(n + 1, log2(charset));
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
		list_store_append_substring(word, path[i], path[i - 1], G, i - j, ls);
		if (i != j) {
			i = j + 1;
		}
	}

	print_rating(entropy, n, label);

	graphviz(G, word, path);

  char graphpath[256];
  sprintf(graphpath, "%s/pwcheck-gtk/pwgraph.svg", g_get_user_cache_dir());
  gtk_image_set_from_file(gi, graphpath);
  remove(graphpath);
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
  GtkLabel            *label_info;
  dictionary          *dict;
  long                dict_words;
	long                dict_nodes;
};

G_DEFINE_TYPE (PwcheckGtkWindow, pwcheck_gtk_window, GTK_TYPE_APPLICATION_WINDOW)


static void
start_computation(PwcheckGtkWindow *self) {
  gchar buf[33];
  strcpy(buf, gtk_entry_get_text(self->te_passwd));
  compute_entropy(buf, self->dict, self->dict_words, self->ls_decomp, self->im_graph, self->label_info);
}

static void
bn_compute_clicked(GtkButton        *button,
                   PwcheckGtkWindow *self)
{
  GTK_IS_BUTTON(button);
  start_computation(self);
}

static void
enter_pressed(GtkEntry         *entry,
              PwcheckGtkWindow *self)
{
  GTK_IS_ENTRY(entry);
  start_computation(self);
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
  gtk_widget_class_bind_template_child (widget_class, PwcheckGtkWindow, label_info);
  gtk_widget_class_bind_template_callback (widget_class, bn_compute_clicked);
  gtk_widget_class_bind_template_callback (widget_class, enter_pressed);
}

static void
pwcheck_gtk_window_init (PwcheckGtkWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  /*
   * init cache directory
   */
  mkdir(g_build_path("/", g_get_user_cache_dir(), "pwcheck-gtk", NULL), 0700);

  /*
   * init dictionary
   */
  const gchar *const *dirs = g_get_system_data_dirs();
  FILE *fd = NULL;
  for (int i = 0; dirs[i]; i++) {
	  fd = fopen(g_build_path("/", dirs[i], "pwcheck-gtk", "dictionary.txt", NULL), "r");
	  if (!fd) {
		  continue;
	  } else {
      printf("Opened: %s\n", g_build_path("/", dirs[i], "pwcheck-gtk", "dictionary.txt", NULL));
      goto go_on;
    }
  }
  fprintf(stderr, "FATAL ERROR: Could not open \"dictionary.txt\".\n");
	exit(-1);

go_on:
	self->dict = dictionary_new(&self->dict_words, &self->dict_nodes, fd);
  fclose(fd),

  /*
   * draw a start screen
   */
  bn_compute_clicked(self->bn_compute, self);
}


