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

#include "dictionary.h"
#include "utils.h"

#include <gtk/gtk.h>



/**
 * category:
 * @NON: no type
 * @RND: random string
 * @WRD: dictionary word
 * @SEQ: sequence
 * @KBP: keyboard pattern
 * @REP: repetition
 *
 * Categories for the different types of substrings.
 */
typedef enum {
  NON,
  WRD,
  SEQ,
  KBP,
  REP,
  RND
} category;



/**
 * graph:
 * @n: size, i.e. number of nodes
 * @edge: weighted edges
 * @cat: category for each edge
 * @path: the shortest path, i.e. path with the least entropy, padded with -1
 * @word: the password
 *
 * Adjacency matrix representing the graph.
 */
typedef struct {
  int n;
  double **edge;
  category **cat;
  int *path;
  const char *word;
} graph;



/**
 * graph_new:
 * @word: the password
 *
 * Creates a new graph from a password.
 *
 * Returns: the graph
 */
graph *graph_new(const char *word);



/**
 * graph_free:
 * @G: the graph
 *
 * Frees a graph's memory.
 */
void graph_free(graph *G);



/**
 * graph_update_edge:
 * @G: the graph
 * @u: starting node of the edge
 * @v: end node of the edge
 * @weight: weight of the edge (i.e. entropy)
 * @cat: category of the edge
 *
 * Updates a graph edge, if the new weight is less than the previous weight.
 */
void graph_update_edge(graph *G, int u, int v, double weight, category cat);



/**
 * graph_compute_edges:
 * @G: the graph
 * @dict: the dictionary
 *
 * Computes the edges in a graph for a password
 */
void graph_compute_edges(graph *G, dictionary *dict);



/**
 * graph_compute_path:
 * @G: the graph
 *
 * Computes the shortes path from node 1 to node n using Dijkstra's algorithm.
 *
 * Returns: The path's combined entropy.
 */
double graph_compute_path(graph *G);



/**
 * graph_gio_print_dot:
 * @G: the graph
 * @stream: the stream to print the dot output to
 *
 * Prints out a graph in dot format to a file.
 */
void graph_gio_print_dot(graph *G, GOutputStream  *stream);



/**
 * graph_gio_get_svg:
 * @G: the graph to paint
 *
 * TODO: implement without cached file
 * TODO: make dot work in Flatpak
 */
GInputStream *graph_gio_get_svg(graph *G);





