/* graph.c
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

#include "graph.h"

#include <math.h>



graph *
graph_new(const gchar *word)
{
  graph *G = malloc(sizeof(graph));
  G->n = strlen(word) + 1;
  G->edge = malloc(G->n * sizeof(gdouble *));
  G->cat = malloc(G->n * sizeof(category *));
  G->path = malloc(G->n * sizeof(gint));
  G->word = word;
  gdouble weight = log2(compute_charset(word));
  for (gint u = 0; u < G->n; u++) {
    G->edge[u] = malloc(G->n * sizeof(gdouble));
    G->cat[u] = malloc(G->n * sizeof(category));
    G->path[u] = -1;
    for (gint v = 0; v < G->n; v++) {
      if (u < G->n - 1 && u + 1 == v) {
        G->edge[u][v] = weight;
        G->cat[u][v] = RND;
      } else {
        G->edge[u][v] = INFINITY;
        G->cat[u][v] = NON;
      }
    }
  }
  return G;
}



void
graph_free(graph *G)
{
  if (G) {
    for (gint u = 0; u < G->n; u++) {
      free(G->edge[u]);
      free(G->cat[u]);
    }
    free(G->edge);
    free(G->cat);
    free(G->path);
    free(G);
  }
}



void
graph_update_edge(graph    *G,
                  gint       u,
                  gint       v,
                  gdouble    weight,
                  category  cat)
{
  if (G->edge[u][v] > weight) {
    G->edge[u][v] = weight;
    G->cat[u][v] = cat;
  }
}



gdouble
graph_compute_path(graph *G)
{
  // dist[u]: distance from node u to node n
  gdouble dist[G->n];
  // dist[u]: predecessor of node n
  gint pred[G->n];
  for (gint i = 0; i < G->n; i++) {
    dist[i] = INFINITY;
    pred[i] = -1;
  }
  dist[0] = 0;
  for (gint u = 0; u < G->n; u++) {
    for (gint v = 0; v < G->n; v++) {
      if (G->edge[u][v] != INFINITY) {
        if (dist[v] > dist[u] + G->edge[u][v]) {
          dist[v] = dist[u] + G->edge[u][v];
          pred[v] = u;
        }
      }
    }
  }
  gint j = 0;
  gint buf[G->n];
  for (gint i = G->n - 1; i >= 0; i = pred[i], j++) {
    buf[j] = i;
  }
  for ( ; j < G->n; j++) {
    buf[j] = -1;
  }
  for (gint i = 0; i < G->n; i++) {
    G->path[i] = buf[G->n - i - 1];
  }
  /* shortest path's entropy */
  return dist[G->n - 1];
}



void graph_gio_print_dot(graph *G, GOutputStream  *stream)
{
  g_output_stream_printf(stream, NULL, NULL, NULL, "digraph G {\n"
                "\tgraph [bgcolor=\"transparent\"];\n"
                "\tedge [fontname = \"cantarell\", fontsize=10];\n"
                "\tnode [shape = circle, fontname = \"cantarell\", fontsize=10];\n"
                "\trankdir = LR;\n");
  for (gint u = 0; u < G->n; u++) {
    for (gint v = 0; v < G->n; v++) {
      if (G->edge[u][v] < INFINITY) {
        gchar buf[strlen(G->word) * 2 + 1];
        gint i;
        const gchar *c;
        for (i = 0, c = G->word + u; i < v - u; i++, c++) {
          if (*c == '\"') {
            buf[i] = '\\';
            buf[++i] = '\"';
          } else {
            buf[i] = *c;
          }
        }
        buf[i] = '\0';
        gchar *style = "";
        for (gint i = 0; i < G->n - 1; i++) {
          if (G->path[i] == u && G->path[i + 1] == v) {
            style = ", penwidth=3";
          }
        }
        gchar *color = "";
        switch (G->cat[u][v]) {
          case WRD:
            color = ", color=red";
            break;
          case SEQ:
            color = ", color=green";
            break;
          case KBP:
            color = ", color=blue";
            break;
          case REP:
            color = ", color=orange";
            break;
          default:
            break;
        }
        g_output_stream_printf(stream, NULL, NULL, NULL, "\t%d -> %d [ label = \"%s: %.1f\"%s%s ];\n", u, v, buf, G->edge[u][v], color, style);
      }
    }
  }
  g_output_stream_printf(stream, NULL, NULL, NULL, "}\n");
}



GInputStream *
graph_gio_get_svg(graph *G)
{
  GSubprocess *dot = g_subprocess_new(G_SUBPROCESS_FLAGS_STDIN_PIPE
                                      | G_SUBPROCESS_FLAGS_STDOUT_PIPE,
                                      NULL,
                                      "dot",
                                      "-Tsvg",
                                      NULL);
  GInputStream *dotin = g_subprocess_get_stdout_pipe(dot);
  GOutputStream *dotout = g_subprocess_get_stdin_pipe(dot);
  graph_gio_print_dot(G, dotout);
  g_output_stream_close(dotout, NULL, NULL);
  return dotin;
}



void
graph_compute_edges(graph      *G,
                    dictionary *dict)
{
  dictionary *repetitions = dict_new();
  for (const char *c = G->word; *c != '\0'; c++) {
    gsize seqlen = find_seq(c);
    gsize kbplen = find_kbp(c);
    gsize wrdlen[G->n];
    gint wrdcnt = dict_find_wrd(dict, c, wrdlen);
    gsize replen[G->n];
    gint repcnt = dict_find_wrd(repetitions, c, replen);

    for (gsize i = 3; i <= seqlen; i++) {
      graph_update_edge(G, c - G->word, c - G->word + i, rate_seq(*c, i), SEQ);
    }
    for (gsize i = 3; i <= kbplen; i++) {
      graph_update_edge(G, c - G->word, c - G->word + i, rate_kbp(i), KBP);
    }
    for (gint i = 0; i < wrdcnt; i++) {
      graph_update_edge(G, c - G->word, c - G->word + wrdlen[i], dict_rate_wrd(dict), WRD);
    }
    for (gint i = 0; i < repcnt; i++) {
      graph_update_edge(G, c - G->word, c - G->word + replen[i], dict_rate_wrd(repetitions), REP);
    }

    for (int i = 0; i < c - G->word; i++) {
      gchar rep[c - G->word - i + 1];
      strncpy(rep, G->word + i, c - G->word - i + 1);
      rep[c - G->word - i + 1] = '\0';
      dict_add_word(repetitions, rep);
    }
  }
  dict_free(repetitions);
}



