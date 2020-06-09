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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "graph.h"



/*
 *  Allocates and returns a new graph.
 *
 *  int n		number of nodes
 *  double value	default value for the edges on the path (1, 2, ..., n)
 */
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



/*
 *  Frees a graph's memory.
 */
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



/*
 *  Updates a graph edge, if the new weight is less than the previous weight.
 */
void graph_update_edge(graph *G, int u, int v, double weight, category cat) {
	if (G->edge[u][v] > weight) {
		G->edge[u][v] = weight;
		G->cat[u][v] = cat;
	}
}



/*
 *  Computes the shortes path from node 1 to node n.
 *
 *  int *path	string array of size n containing the path
 *		All empty fields are set to -1.
 */
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



/*
 *  Prints out a graph.
 */
void graph_print(FILE *file, graph *G, char *str, int *path) {
	fprintf(file, "digraph G {\n");
	fprintf(file, "\tnode [shape = circle];\n");
	fprintf(file, "\trankdir = LR;\n");
	for (int u = 0; u < G->n; u++) {
		for (int v = 0; v < G->n; v++) {
			if (G->edge[u][v] < INFINITY) {
				char buf[strlen(str) + 1];
				strncpy(buf, str + u, v - u);
				buf[v - u] = '\0';
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





void graphviz(graph *G, char *str, int *path) {
	//printf("<p>\n<img style=\"width:100%%\" src='data:image/svg+xml;utf8,");
	fflush(stdout);
	FILE *dot = popen("dot -Tsvg > /home/stephan/.pwgraph.svg", "w");
	graph_print(dot, G, str, path);
	//printf("' />\n</p>\n");
	fclose(dot);
}

