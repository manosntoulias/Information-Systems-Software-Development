//pam_metrics.h

#ifndef PAM_METRICS_H_
#define PAM_METRICS_H_

#include "pam_graph.h"

//1. Degree Distribution
void degreeDistribution(pam_graph *g);

//2. Diameter
int diameter(pam_graph *g);

//3. Average Path Length
double averagePathLength(pam_graph *g);

//4. Number Of CCs
int numberOfCCs(pam_graph *g);

//5. Max CC
int maxCC(pam_graph* g);

//6. Density
double density(pam_graph *g);

//7. Centrality
//A. Closeness Centrality
double pam_closenessCentrality(pam_node* n, pam_graph* g);
	
//B. Betweenness Centrality
double pam_betweennessCentrality(pam_node* n, pam_graph* g);

#endif
