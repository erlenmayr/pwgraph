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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>



graph *
graph_new(char *word)
{
  graph *G = malloc(sizeof(graph));
  G->n = strlen(word) + 1;
  double weight = log2(compute_charset(word));
  G->edge = malloc(G->n * sizeof(double *));
  G->cat = malloc(G->n * sizeof(category *));
  G->path = malloc(G->n * sizeof(int));
  G->word = word;
  for (int u = 0; u < G->n; u++) {
    G->edge[u] = malloc(G->n * sizeof(double));
    G->cat[u] = malloc(G->n * sizeof(category));
    G->path[u] = -1;
    for (int v = 0; v < G->n; v++) {
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
    if (G->edge) {
      for (int u = 0; u < G->n; u++) {
        free(G->edge[u]);
      }
      free(G->edge);
    }
    if (G->cat) {
      for (int u = 0; u < G->n; u++) {
        free(G->cat[u]);
      }
      free(G->cat);
    }
    if (G->path) {
      free(G->path);
    }
    free(G);
  }
}



void
graph_update_edge(graph    *G,
                  int       u,
                  int       v,
                  double    weight,
                  category  cat)
{
  if (G->edge[u][v] > weight) {
    G->edge[u][v] = weight;
    G->cat[u][v] = cat;
  }
}



double
graph_compute_path(graph *G)
{
  // dist[u]: distance from node u to node n
  double dist[G->n];
  // dist[u]: predecessor of node n
  int pred[G->n];
  for (int i = 0; i < G->n; i++) {
    dist[i] = INFINITY;
    pred[i] = -1;
  }
  dist[0] = 0;
  for (int u = 0; u < G->n; u++) {
    for (int v = 0; v < G->n; v++) {
      if (G->edge[u][v] != INFINITY) {
        if (dist[v] > dist[u] + G->edge[u][v]) {
          dist[v] = dist[u] + G->edge[u][v];
          pred[v] = u;
        }
      }
    }
  }
  int j = 0;
  int buf[G->n];
  for (int i = G->n - 1; i >= 0; i = pred[i], j++) {
    buf[j] = i;
  }
  for ( ; j < G->n; j++) {
    buf[j] = -1;
  }
  for (int i = 0; i < G->n; i++) {
    G->path[i] = buf[G->n - i - 1];
  }
  /* shortest path's entropy */
  return dist[G->n - 1];
}



void
graph_print_dot(graph *G,
                FILE  *file)
{
  fprintf(file, "digraph G {\n"
                "\tgraph [bgcolor=\"transparent\"];\n"
                "\tedge [fontname = \"cantarell\", fontsize=10];\n"
                "\tnode [shape = circle, fontname = \"cantarell\", fontsize=10];\n"
                "\trankdir = LR;\n");
  for (int u = 0; u < G->n; u++) {
    for (int v = 0; v < G->n; v++) {
      if (G->edge[u][v] < INFINITY) {
        char buf[strlen(G->word) * 2 + 1];
        int i;
        char *c;
        for (i = 0, c = G->word + u; i < v - u; i++, c++) {
          if (*c == '\"') {
            buf[i] = '\\';
            buf[++i] = '\"';
          } else {
            buf[i] = *c;
          }
        }
        buf[i] = '\0';
        char *style = "";
        for (int i = 0; i < G->n - 1; i++) {
          if (G->path[i] == u && G->path[i + 1] == v) {
            style = ", penwidth=3";
          }
        }
        char *color = "";
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
        fprintf(file, "\t%d -> %d [ label = \"%s: %.1f\"%s%s ];\n", u, v, buf, G->edge[u][v], color, style);
      }
    }
  }
  fprintf(file, "}\n");
}



void
graph_save_svg(graph *G,
               char  *graphfile)
{
  char *command = g_strdup_printf("dot -Tsvg > %s", graphfile);
  FILE *dot = popen(command, "w");
  graph_print_dot(G, dot);
  fclose(dot);
  g_free(command);
}



void
graph_compute_edges(graph      *G,
                    dictionary *dict)
{
  dictionary *repetitions = dict_new();
  for (char *c = G->word; *c != '\0'; c++) {
    int seqlen = find_seq(c);
    int kbplen = find_kbp(c);
    int n = strlen(G->word);
    int wrdlen[n];
    int wrdcnt = dict_find_wrd(dict, c, wrdlen);
    int replen[n];
    int repcnt = dict_find_wrd(repetitions, c, replen);

    for (int i = 3; i <= seqlen; i++) {
      graph_update_edge(G, c - G->word, c - G->word + i, rate_seq(*c, i), SEQ);
    }
    for (int i = 3; i <= kbplen; i++) {
      graph_update_edge(G, c - G->word, c - G->word + i, rate_kbp(i), KBP);
    }
    for (int i = 0; i < wrdcnt; i++) {
      graph_update_edge(G, c - G->word, c - G->word + wrdlen[i], dict_rate_wrd(dict), WRD);
    }
    for (int i = 0; i < repcnt; i++) {
      graph_update_edge(G, c - G->word, c - G->word + replen[i], dict_rate_wrd(repetitions), REP);
    }

    for (int i = 0; i < c - G->word; i++) {
      char rep[c - G->word - i + 1];
      strncpy(rep, G->word + i, c - G->word - i + 1);
      rep[c - G->word - i + 1] = '\0';
      dict_add_word(repetitions, rep);
    }
  }
  dict_free(repetitions);
}



