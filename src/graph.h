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
 * @path: the shortest path, i.e. path with least entropy
 *
 * Adjacency matrix representing the graph.
 */
typedef struct {
  int n;
  double **edge;
  category **cat;
  int *path;
} graph;



/**
 * graph_new:
 * @n: number of nodes
 * @weight: default value for the edges on the path (1, 2, ..., n)
 *
 * Allocates and returns a new graph.
 */
graph *graph_new(int n, double weight);



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
 * graph_compute_path:
 * @G: the graph
 * @path: integer array representing the path
 *
 * Computes the shortes path from node 1 to node n.
 *
 * Returns: The path's combined entropy.
 */
double graph_compute_path(graph *G, int *path);



/**
 * graph_print_dot:
 * @G: the graph
 * @file: the stream to print the dot output to
 * @word: the password for edge tags
 * @path: the path to highlight
 *
 * Prints out a graph in dot format to a file.
 */
void graph_print_dot(graph *G, FILE *file, char *word, int *path);



/**
 * graphviz:
 * @G: the graph to paint
 * @word: the password
 * @path: the path on the graph to highlight
 * @graphfile: path of the graph SVG image
 *
 * TODO: implement without cached file
 * TODO: make dot work in Flatpak
 */
void graph_save_svg(graph *G, char *word, int *path, char *graphfile);



