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

#include "pwcheck_gtk-window.h"
#include "pwcheck_gtk-config.h"
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



/**
 * list_store_set_decomposition:
 * @ls: list store to put the decomposition into
 * @G: the password graph
 *
 *  Puts the decomposition of the graph's shortest path into a GtkListStore
 */
void
list_store_set_decomposition(GtkListStore *ls,
                             graph        *G)
{
  GTK_IS_LIST_STORE(ls);
  gtk_list_store_clear(ls);
  for (gint i = 0; i < G->n - 1; i++) {
    if (G->path[i] < 0) {
      continue;
    }
    GtkTreeIter iter;
    gint len = G->path[i + 1] - G->path[i];
    gchar *buf = g_strndup(G->word + G->path[i], len);
    gtk_list_store_append(ls, &iter);
    gtk_list_store_set(ls, &iter,
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
 * label_set_rating:
 * @label: label to put the message to
 * @entropy: entropy result of the password
 * @len: length of the password
 *
 * Prints information about the password to a GtkLabel.
 */
void
label_set_rating(GtkLabel *label,
                 gdouble    entropy,
                 gint       len)
{
  GTK_IS_LABEL(label);
  gchar *buf = g_strdup_printf("You password entropy: %.1f bits\n"
                              "Maximum entropy for ASCII passwords of length %d: %.1f bits\n",
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
 * @word: the password
 * @graphfile: path of the graph SVG image
 *
 * Computes the entropy of the password.
 */
GInputStream *
compute_entropy(GtkListStore *ls,
                GtkLabel     *label,
                dictionary   *dict,
                const gchar  *word)
{
  graph *G = graph_new(word);
  graph_compute_edges(G, dict);
  gdouble entropy =  graph_compute_path(G);
  list_store_set_decomposition(ls, G);
  label_set_rating(label, entropy, G->n - 1);
  GInputStream *svg = graph_gio_get_svg(G);
  graph_free(G);
  return svg;
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
  GtkButton           *bn_about;
  GtkAboutDialog      *about;
  GtkScrolledWindow   *sw_graph;
  dictionary          *dict;
};

G_DEFINE_TYPE(PwcheckGtkWindow, pwcheck_gtk_window, GTK_TYPE_APPLICATION_WINDOW)



static void
draw_graph(PwcheckGtkWindow *self, GInputStream *svg)
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
start_computation(PwcheckGtkWindow *self)
{
  if (g_str_is_ascii(gtk_entry_get_text(self->te_passwd))) {
    GInputStream *svg = compute_entropy(self->ls_decomp, self->label_info, self->dict, gtk_entry_get_text(self->te_passwd));
    draw_graph(self, svg);
  } else {
    gtk_label_set_text(self->label_info, "Only ASCII characters are supported.");
  }

}



/*
 * Widget callback functions.
 */
static void
window_resized(GtkWidget        *sw,
               PwcheckGtkWindow *self)
{
  GTK_IS_WIDGET(sw);
  GTK_IS_WIDGET(self);
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
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, sw_graph);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, label_info);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, about);
  gtk_widget_class_bind_template_child(widget_class, PwcheckGtkWindow, bn_about);
  gtk_widget_class_bind_template_callback(widget_class, bn_compute_clicked);
  gtk_widget_class_bind_template_callback(widget_class, enter_pressed);
  gtk_widget_class_bind_template_callback(widget_class, bn_about_clicked);
  gtk_widget_class_bind_template_callback(widget_class, window_resized);
  gtk_widget_class_bind_template_callback(widget_class, gtk_widget_hide_on_delete);
}

static void
pwcheck_gtk_window_init(PwcheckGtkWindow *self)
{
  gtk_widget_init_template(GTK_WIDGET(self));

  gtk_about_dialog_set_version(self->about, PACKAGE_VERSION);
  /*
   * Init dictionary.
   */
  GInputStream *stream = g_resources_open_stream("/com/verbuech/pwcheck-gtk/dictionary.txt",
                          G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
  self->dict = dict_new_from_stream(stream);
  g_input_stream_close(stream, NULL, NULL);
  g_object_unref(stream);
}



