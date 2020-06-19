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

#include "dictionary.h"
#include "patterns.h"
#include "graph.h"

#include <math.h>



/*
 * following category enum
 */
const char *name[] = {
  "none",
  "dictionary",
  "sequence",
  "keyboard",
  "repetition",
  "random"
};



/*
 * following category enum
 * N/A should never appear
 */
const char *formula[] = {
  "N/A",
  "log₂(𝑑)",
  "log₂(𝑐) + (𝑛 - 1) log₂(3)",
  "log₂(𝑘) + (𝑛 - 1) log₂(9)",
  "log₂(1 + ½ 𝑖 (𝑖 + 1))",
  "𝑛 log₂(𝑐)"
};



/**
 * list_store_append_substring:
 * @ls: list store to append the result to
 * @G: the whole graph
 * @word: the whole password string for output purposes
 * @u: the node that starts the substring
 * @v: the node that ends the substring
 * @pathlen: the number of random characters in a row
 *
 *  Prints the result line for a substring starting at u, ending at v
 */
void
list_store_append_substring(GtkListStore *ls,
                            graph        *G,
                            const char   *word,
                            int           u,
                            int           v,
                            int           pathlen)
{
  GTK_IS_LIST_STORE(ls);
  double entropy = G->edge[u][v];
  int sublen = v - u;
  if (G->cat[u][v] == RND) {
    sublen = pathlen;
    entropy *= pathlen;
  }
  char substring[sublen + 1];
  strncpy(substring, word + u, sublen);
  substring[sublen] = '\0';

  GtkTreeIter iter;
  gtk_list_store_append(ls, &iter);
  gtk_list_store_set(ls, &iter,
                     0, substring,
                     1, name[G->cat[u][v]],
                     2, sublen,
                     3, formula[G->cat[u][v]],
                     4, entropy,
                     -1);
}



/**
 * print_rating:
 * @label: label to put the message to
 * @entropy: entropy result of the password
 * @len: length of the password
 *
 * Prints information about the password to a GtkLabel.
 */
void
label_set_rating(GtkLabel *label,
                 double    entropy,
                 int       len)
{
  GTK_IS_LABEL(label);
  char *buf = g_strdup_printf("You password entropy: %.1f bits\n"
                              "Maximum entropy for length %d: %.1f bits\n",
                              entropy,
                              len,
                              len * log2(94));
  gtk_label_set_text(label, buf);
  g_free(buf);
}



/**
 * compute_entropy:
 * @ls: the list store for the result
 * @label: the label for the summary
 * @dict: the dictionary
 * @dict_words: number of words in the dictionary (for entropy calculation)
 * @word: the password
 * @graphfile: path of the graph SVG image
 *
 * Computes and returns the entropy of the password.
 */
double
compute_entropy(GtkListStore *ls,
                GtkLabel     *label,
                dictionary   *dict,
                char         *word,
                gchar        *graphfile)
{
  int n = strlen(word);
  int charset = compute_charset(word);
  graph *G = graph_new(n + 1, log2(charset));

  graph_compute_edges(G, dict, word);


  int path[G->n];
  double entropy = graph_compute_path(G, path);

  gtk_list_store_clear(ls);
  int i;
  for (i = G->n - 1; path[i] == -1; i--);
  for (; i > 0; i--) {
    int j;
    for (j = i; j > 0 && path[j - 1] == path[j] + 1; j--);
    list_store_append_substring(ls, G, word, path[i], path[i - 1], i - j);
    if (i != j) {
      i = j + 1;
    }
  }

  label_set_rating(label, entropy, n);

  graph_save_svg(G, word, path, graphfile);
  graph_free(G);

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
  gchar               *graphfile;
  GtkButton           *bn_about;
  GtkWidget           *about;
  GtkScrolledWindow   *sw_graph;
  GtkViewport         *vp_graph;
};

G_DEFINE_TYPE(PwcheckGtkWindow, pwcheck_gtk_window, GTK_TYPE_APPLICATION_WINDOW)



static void
draw_graph(PwcheckGtkWindow *self)
{
  GError *err = NULL;
  gint sf = gdk_window_get_scale_factor(gtk_widget_get_window(GTK_WIDGET(self)));
  GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_scale(self->graphfile,
                                                    sf * gtk_widget_get_allocated_width(GTK_WIDGET(self->sw_graph)),
                                                    sf * gtk_widget_get_allocated_height(GTK_WIDGET(self->sw_graph)),
                                                    TRUE,
                                                    &err);
  if (err) {
    fprintf(stderr, "ERROR loading file: %s\n", err->message);
    g_object_unref(err);
  }
  cairo_surface_t *cs = gdk_cairo_surface_create_from_pixbuf(pb, 0, gtk_widget_get_window(GTK_WIDGET(self)));
  gtk_image_set_from_surface(self->im_graph, cs);
  g_object_unref(pb);
}



static void
start_computation(PwcheckGtkWindow *self)
{
  gchar buf[gtk_entry_get_text_length(self->te_passwd) + 1];
  strcpy(buf, gtk_entry_get_text(self->te_passwd));
  if (g_str_is_ascii(buf)) {
    compute_entropy(self->ls_decomp, self->label_info, self->dict, buf, self->graphfile);
  } else {
    gtk_label_set_text(self->label_info, "Only ASCII characters are supported.");
  }

}



/*
 *  widget callback functions
 */
static void
window_resized(GtkWidget        *sw,
               PwcheckGtkWindow *self)
{
  GTK_IS_WIDGET(sw);
  draw_graph(self);
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
bn_about_clicked(GtkButton        *button,
                 PwcheckGtkWindow *self)
{
  GTK_IS_BUTTON(button);
  gtk_widget_show(GTK_WIDGET(self->about));
}

static void
on_about_closed(GtkButton         *button,
                 PwcheckGtkWindow *self)
{
  GTK_IS_BUTTON(button);
  gtk_widget_hide(GTK_WIDGET(self->about));
}



static void
pwcheck_gtk_window_class_init(PwcheckGtkWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  gtk_widget_class_set_template_from_resource(widget_class, "/com/verbuech/pwcheck-gtk/pwcheck_gtk-window.ui");

  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, header_bar);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, bn_compute);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, te_passwd);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, ls_decomp);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, tv_decomp);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, im_graph);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, vp_graph);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, sw_graph);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, label_info);
  /* TODO: make proper menu popover */
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, about);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, bn_about);
  gtk_widget_class_bind_template_callback(widget_class, bn_compute_clicked);
  gtk_widget_class_bind_template_callback(widget_class, enter_pressed);
  gtk_widget_class_bind_template_callback(widget_class, bn_about_clicked);
  gtk_widget_class_bind_template_callback(widget_class, on_about_closed);
  gtk_widget_class_bind_template_callback(widget_class, window_resized);
}

static void
pwcheck_gtk_window_init(PwcheckGtkWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  /*
   * init cache directory
   */
  gchar *cachedir = g_build_path("/", g_get_user_cache_dir(), "pwcheck-gtk", NULL);
  g_mkdir_with_parents(cachedir, 0700);
  g_free(cachedir);
  self->graphfile = g_build_path("/", g_get_user_cache_dir(), "pwcheck-gtk", "pwgraph.svg", NULL);

  /*
   * init dictionary
   * TODO: make this work in ~/.cache/gnome-builder/install
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
  fprintf(stderr, "FATAL ERROR: Could not find “dictionary.txt”.\n");
  exit(-1);

go_on:
  self->dict = dict_new_from_file(fd);
  fclose(fd);

  /*
   * draw a start screen
   */
  start_computation(self);
}



