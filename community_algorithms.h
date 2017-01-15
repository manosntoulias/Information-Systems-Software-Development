//community_algorithms.h

#include "pam_graph.h"

#ifndef COMMUNITY_ALGORITHMS
#define COMMUNITY_ALGORITHMS

//STRUCTS: Communities
typedef struct Community{
	int size;
	int** data;
	}Community;

typedef struct Communities{
	int size;
	int clique_size;
	Community** data;
	}Communities;
	
typedef struct gn_Community{
	int size;
	int* data;
	} gn_Community;

	
typedef struct gn_Communities{
	int size;
	gn_Community* data;
	} gn_Communities;

Communities* cliquePercolationMethod(int k, pam_graph* my_graph);

gn_Communities* Girvan_Newman(pam_graph *g, double limit_modularity);

#endif
