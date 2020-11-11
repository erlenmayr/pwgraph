/* pwgraph-window.c
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

#include "pwgraph-window.h"
#include "pwgraph-config.h"
#include "graph.h"
#include <math.h>



/*
 * following category enum
 */
const gchar *name[] = {
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
const gchar *formula[] = {
  "N/A",
  "log₂(𝑑)",
  "log₂(𝑐) + (𝑛 - 1) log₂(3)",
  "log₂(𝑘) + (𝑛 - 1) log₂(9)",
  "log₂(1 + ½ 𝑖 (𝑖 + 1))",
  "𝑛 log₂(𝑐)"
};



struct _PwgraphWindow
{
  GtkApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkButton           *bn_compute;
  GtkEntry            *te_passwd;
  GtkListStore        *ls_decomp;
  GtkTreeView         *tv_decomp;
  GtkImage            *im_graph;
  GtkLabel            *label_pwv;
  GtkLabel            *label_mxv;
  GtkButton           *bn_about;
  GtkAboutDialog      *about;
  GtkScrolledWindow   *sw_graph;
  dictionary          *dict;
};

G_DEFINE_TYPE (PwgraphWindow, pwgraph_window, GTK_TYPE_APPLICATION_WINDOW)



/**
 * list_store_set_decomposition:
 * @ls: list store to put the decomposition into
 * @G: the password graph
 *
 *  Puts the decomposition of the graph's shortest path into a GtkListStore
 */
void
list_store_set_decomposition(PwgraphWindow *self,
                             graph            *G)
{
  GTK_IS_LIST_STORE(self->ls_decomp);
  gtk_list_store_clear(self->ls_decomp);
  for (gint i = 0; i < G->n - 1; i++) {
    if (G->path[i] < 0) {
      continue;
    }
    GtkTreeIter iter;
    gint len = G->path[i + 1] - G->path[i];
    gchar *buf = g_strndup(G->word + G->path[i], len);
    gtk_list_store_append(self->ls_decomp, &iter);
    gtk_list_store_set(self->ls_decomp, &iter,
                       0, buf,
                       1, name[G->cat[G->path[i]][G->path[i + 1]]],
                       2, len,
                       3, formula[G->cat[G->path[i]][G->path[i + 1]]],
                       4, G->edge[G->path[i]][G->path[i + 1]],
                       -1);
    g_free(buf);
  }
}



/**
 * compute_entropy:
 * @ls: the list store for the result
 * @label: the label for the summary
 * @dict: the dictionary
 * @word: the password
 * @graphfile: path of the graph SVG image
 *
 * Computes the entropy of the password.
 */
GInputStream *
compute_entropy(PwgraphWindow *self,
                const gchar      *word)
{
  graph *G = graph_new(word);
  graph_compute_edges(G, self->dict);
  gdouble entropy =  graph_compute_path(G);
  list_store_set_decomposition(self, G);
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  gtk_label_set_text(self->label_pwv, g_ascii_dtostr(buf, G_ASCII_DTOSTR_BUF_SIZE, entropy));
  gtk_label_set_text(self->label_mxv, g_ascii_dtostr(buf, G_ASCII_DTOSTR_BUF_SIZE, (G->n - 1) * log2(94)));
  GInputStream *svg = graph_gio_get_svg(G);
  graph_free(G);
  return svg;
}



/*
 * All the steps that has to be done to get the SVG to a GtkImage widget.
 * Some steps are required to include HiDPI scale factor.
 */
static void
draw_graph(PwgraphWindow *self,
           GInputStream     *svg)
{
  /* scale factor for HiDPI screens */
  gint sf = gdk_window_get_scale_factor(gtk_widget_get_window(GTK_WIDGET(self)));
  GdkPixbuf *pb = gdk_pixbuf_new_from_stream_at_scale(svg,
                                                    sf * gtk_widget_get_allocated_width(GTK_WIDGET(self->sw_graph)),
                                                    sf * gtk_widget_get_allocated_height(GTK_WIDGET(self->sw_graph)),
                                                    TRUE,
                                                    NULL,
                                                    NULL);
  g_input_stream_close(svg, NULL, NULL);
  g_object_unref(svg);
  cairo_surface_t *cs = gdk_cairo_surface_create_from_pixbuf(pb, 0, gtk_widget_get_window(GTK_WIDGET(self)));
  gtk_image_set_from_surface(self->im_graph, cs);
  cairo_surface_destroy(cs);
  g_object_unref(pb);
}



/*
 * Separate function because computation can be triggered multiple ways.
 */
static void
start_computation(PwgraphWindow *self)
{
  if (g_str_is_ascii(gtk_entry_get_text(self->te_passwd))) {
    GInputStream *svg = compute_entropy(self, gtk_entry_get_text(self->te_passwd));
    draw_graph(self, svg);
  } else {
    gtk_label_set_text(self->label_pwv, "Only ASCII characters are supported.");
  }

}



/*
 * Widget callback functions.
 */
static void
on_compute_clicked(GtkButton *button,
                   gpointer   userdata)
{
  GTK_IS_BUTTON(button);
  PWGRAPH_IS_WINDOW(userdata);
  start_computation(userdata);

}

static void
on_enter_pressed(GtkEntry *entry,
                 gpointer  userdata)
{
  GTK_IS_ENTRY(entry);
  PWGRAPH_IS_WINDOW(userdata);
  start_computation(userdata);
}

static void
on_about_clicked(GtkButton *button,
                 gpointer   userdata)
{
  GTK_IS_BUTTON(button);
  PWGRAPH_IS_WINDOW(userdata);
  gtk_widget_show(GTK_WIDGET(PWGRAPH_WINDOW(userdata)->about));
}



static void
pwgraph_window_class_init(PwgraphWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  gtk_widget_class_set_template_from_resource(widget_class, "/ch/verbuecheln/pwgraph/pwgraph-window.ui");

  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, header_bar);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, bn_compute);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, te_passwd);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, ls_decomp);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, tv_decomp);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, im_graph);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, sw_graph);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, label_pwv);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, label_mxv);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, about);
  gtk_widget_class_bind_template_child(widget_class, PwgraphWindow, bn_about);
  gtk_widget_class_bind_template_callback(widget_class, on_compute_clicked);
  gtk_widget_class_bind_template_callback(widget_class, on_enter_pressed);
  gtk_widget_class_bind_template_callback(widget_class, on_about_clicked);
  gtk_widget_class_bind_template_callback(widget_class, gtk_widget_hide_on_delete);
}

static void
pwgraph_window_init(PwgraphWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  gtk_about_dialog_set_version(self->about, PACKAGE_VERSION);

  /* Init dictionary. */
  GInputStream *stream = g_resources_open_stream("/ch/verbuecheln/pwgraph/dictionary.txt",
                          G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
  self->dict = dict_new_from_stream(stream);
  g_input_stream_close(stream, NULL, NULL);
  g_object_unref(stream);
}



