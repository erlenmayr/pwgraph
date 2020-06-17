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



graph *graph_new(int n, double weight) {
  graph *G = malloc(sizeof(graph));
  G->n = n;
  G->edge = malloc(n * sizeof(double *));
  G->cat = malloc(n * sizeof(category *));
  for (int u = 0; u < n; u++) {
    G->edge[u] = malloc(n * sizeof(double));
    G->cat[u] = malloc(n * sizeof(category));
    for (int v = 0; v < n; v++) {
      if (u < n - 1 && u + 1 == v) {
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



void graph_free(graph *G) {
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
    free(G);
  }
}



void graph_update_edge(graph *G, int u, int v, double weight, category cat) {
  if (G->edge[u][v] > weight) {
    G->edge[u][v] = weight;
    G->cat[u][v] = cat;
  }
}



double graph_compute_path(graph *G, int *path) {
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
  for (int i = G->n - 1; i >= 0; i = pred[i], j++) {
    path[j] = i;
  }
  for ( ; j < G->n; j++) {
    path[j] = -1;
  }
  double entropy = dist[G->n - 1];
  return entropy;
}



void graph_print(FILE *file, graph *G, char *str, int *path) {
  fprintf(file, "digraph G {\n"
                "\tgraph [bgcolor=\"transparent\"];\n"
                "\tedge [fontname = \"cantarell\", fontsize=10];\n"
                "\tnode [shape = circle, fontname = \"cantarell\", fontsize=10];\n"
                "\trankdir = LR;\n");
  for (int u = 0; u < G->n; u++) {
    for (int v = 0; v < G->n; v++) {
      if (G->edge[u][v] < INFINITY) {
        char buf[strlen(str) * 2 + 1];
        int i;
        char *c;
        for (i = 0, c = str + u; i < v - u; i++, c++) {
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
          if (path[i + 1] == u && path[i] == v) {
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



