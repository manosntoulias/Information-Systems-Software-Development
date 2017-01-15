//pam_utilities.h

#include "pam_graph.h"

#ifndef PAM_UTILITIES
#define PAM_UTILITIES

//UTILITIES: DIAXEIRHSH CCs

void **create_tableOf_g(pam_graph *g, size_t size);

void *table_init(pam_graph *g, size_t size, void** table, int *CC, int endOfCC, void *value);

int get_degree(pam_cell cell);

void destroy_table(pam_graph *g, void **table);

int findCC(int *i, int *j, pam_resultSet *resSet, int *constraints);

int findCCs(pam_graph *g, int *maxCCsize, int *numofCCs, int *constraints, unsigned char **table);

double get_difference(char *birth1, char *birth2);

void split_double(double dbl, int *integer, int *decimal);


//UTILITIES: DIAXEIRHSH ARXEIWN CSV
//Utilities: Load Nodes
void load_person_csv(pam_graph* my_graph, char* filename);

void load_post_csv(pam_graph* my_graph, char* filename);
	
void load_comment_csv(pam_graph* my_graph, char* filename);

void load_forum_csv(pam_graph* my_graph, char* filename);

void load_tag_csv(pam_graph* my_graph, char* filename);


//Utilities: Load Edges
// mode: normal for 0, swaps starting with ending id for 1
void load_relationship_csv(pam_graph* my_graph, char* filename, char mode);

#endif
