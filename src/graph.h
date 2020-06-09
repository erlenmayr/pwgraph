#include <stdio.h>


/*
 *  Categories for the different types of substrings:
 *
 *  NON		no type
 *  RND		random string
 *  WRD		dictionary word
 *  SEQ		sequence
 *  KBP		keyboard pattern
 *  REP		repetition
 */
typedef enum {NON, RND, WRD, SEQ, KBP, REP} category;



// Adjacency matrix representing the graph
typedef struct {
	int n;
	double **edge;
	category **cat;
} graph;



/*
 *  Allocates and returns a new graph.
 *
 *  int n		number of nodes
 *  double value	default value for the edges on the path (1, 2, ..., n)
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
 *  int *path	string array of size n containing the path
 *		All empty fields are set to -1.
 */
double graph_compute_path(graph *G, int *path);



/*
 *  Prints out a graph.
 */
void graph_print(FILE *file, graph *G, char *str, int *path);



/*
 *  Prints out a path.
 *
 *  int n	size of the path array
 */
void path_print(int *path, int n);






void graphviz(graph *G, char *str, int *path);




