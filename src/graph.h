/* graph.h
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

#include <stdio.h>



/*
 *  Categories for the different types of substrings:
 *
 *  NON: no type
 *  RND: random string
 *  WRD: dictionary word
 *  SEQ: sequence
 *  KBP: keyboard pattern
 *  REP: repetition
 */
typedef enum {NON, RND, WRD, SEQ, KBP, REP} category;



/*
 *  Adjacency matrix representing the graph.
 */
typedef struct {
  int n;
  double **edge;
  category **cat;
} graph;



/*
 *  Allocates and returns a new graph.
 *
 *  int n:           number of nodes
 *  double value:    default value for the edges on the path (1, 2, ..., n)
 */
graph *graph_new(int n, double weight);



/*
 *  Frees a graph's memory.
 */
void graph_free(graph *G);



/*
 *  Updates a graph edge, if the new weight is less than the previous weight.
 */
void graph_update_edge(graph *G, int u, int v, double weight, category cat);



/*
 *  Computes the shortes path from node 1 to node n.
 *
 *  int *path:   string array of size n containing the path
 *  All empty:   fields are set to -1.
 */
double graph_compute_path(graph *G, int *path);



/*
 *  Prints out a graph to a file.
 */
void graph_print(FILE *file, graph *G, char *str, int *path);



