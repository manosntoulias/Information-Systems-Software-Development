#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "pam_graph.h"
#include "pam_utilities.h"
#include "defines.h"
#include "community_algorithms.h"


//global data
pam_graph *global_g;
pam_result edge_results[INITIAL_FRINGE_SIZE];
int edge_results_size;
int edge_results_start;
int edge_results_end;
double max_betweenness;
int max_start_id;
int max_end_id;
int quit;
int idle_counter;
pthread_mutex_t *mutex;
pthread_mutex_t mutex_max = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_finish = PTHREAD_COND_INITIALIZER;


void init_global_variables() {
	
	edge_results_size = 0;
	edge_results_start = 0;
	edge_results_end = 0;
	max_betweenness = 0;
	max_start_id = -1;
	max_end_id = -1;
	quit = 0;
	idle_counter = 0;
	return;
}

void pam_init_resSet(pam_resultSet *resSet, pam_cell *cell) {
	// initiate resultSet
	resSet->my_fringe->the_fringe[0] = cell;
	resSet->my_fringe->the_fringe_counter = 1;
	resSet->my_fringe->temp_fringe_counter = 0;
	resSet->my_fringe->visited[0] = cell->my_node->id;
	resSet->my_fringe->visited_counter = 1;
	resSet->dist = resSet->current_node = 0;
}


bool pam_destroy_edge(pam_graph *g, int start_id, int end_id, node_type type) {
	//destroys a single edge from graph (including all of the relationships)
	// ONE DIRECTION ONLY (<3)
	pam_cell *start_cell;
	pam_lite_edge *edge, *prev_edge;
	
	//find the cell of starting node
	if ((start_cell = pam_lookup_cell(g, start_id)) == NULL)
	{	printf("by lookup_cell\n");
		return false;
	}
	
	//find the edge of that node
	edge = start_cell->my_list[type].my_edges;
	//akraia periptwsh: to id sthn arxh ths listas
	if (edge->ending_id == end_id)
	{	start_cell->my_list[type].my_edges = edge->next_pam_edge;
	}
	else
	{	prev_edge = edge;
		edge = edge->next_pam_edge;
		while (edge->ending_id != end_id)
		{	prev_edge = edge;
			edge = edge->next_pam_edge;
		}
		prev_edge->next_pam_edge = edge->next_pam_edge;
	}
	//delete the edge
	pam_free_edge(edge);
	start_cell->my_list[type].num_of_edges--;
	g->num_of_edges--;
	
	return true;
}

void pam_init_edgeSet(pam_resultSet *resSet, int end_id) {
	int i;
	pam_cell *temp_cell;
	
	set_constraints(resSet, NULL, 2);
	expand_fringe(resSet->my_graph, resSet->my_fringe, NULL, 2);
	resSet->dist = 1;
	resSet->current_node = 0;
	
	
	for (i=1; i < resSet->my_fringe->the_fringe_counter; i++)
	{	resSet->my_fringe->the_weight[i] = 1.0 / DECIMAL_DIGITS;
		if (resSet->my_fringe->the_fringe[i]->my_node->id == end_id)
		{	
			temp_cell = resSet->my_fringe->the_fringe[i];
			resSet->my_fringe->the_fringe[i] = resSet->my_fringe->the_fringe[0];
			resSet->my_fringe->the_fringe[0] = temp_cell;
		}
	}
}

pam_D_result *pam_reachEdges(pam_graph *graph, int start_id, int end_id, double *centrality, int *size) {
	pam_resultSet *startSet;
	pam_result res;
	int start_counter = 0, i, valid_paths, nonvalid;
	pam_D_result *start_res;
	
	if ((startSet = pam_reachNodesN(graph, start_id)) == NULL)
	{	printf("by reachNodesN\n");
		return NULL;
	}
	
	//arxikopoihsh twn set
	pam_init_edgeSet(startSet, end_id);
			
	while (pam_next(startSet, &res))
	{	start_counter++;	
	}		
	
	//epanaarxikopoihsh gia epanalhpsh reachnodesN
	pam_init_resSet(startSet, pam_lookup_cell(graph, start_id));
	pam_init_edgeSet(startSet, end_id);
	
	if ((start_res = malloc(start_counter*sizeof(pam_D_result))) == NULL)
	{	printf("Cannot allocate memory for start_results\n");
		return NULL;
	}
	
	i=0;
	while (pam_next(startSet, &res))
	{	start_res[i].p_m.id = res.id;
		start_res[i].p_m.distance = res.distance;
		start_res[i].weight = startSet->my_fringe->the_weight[startSet->current_node-1];
		split_double(start_res[i].weight, &valid_paths, &nonvalid);
		//printf("id: %d dist: %d\n", res.id, res.distance);
		*centrality += (double)valid_paths/nonvalid;
		i++;
	}
	
	pam_destroy_resultSet(startSet);
	
	//return the table alloced and its size
	*size = start_counter;
	return start_res;
}

double edgeBetweenness(pam_graph *g, int start_id, int end_id) {
	int i, j, start_size, end_size;
	pam_D_result *start_res, *end_res;
	double start_c=0, end_c=0, centrality=0; //centrality
	pam_resultSet *resSet;
	pam_result res;
	char found;
	
	if ((start_res = pam_reachEdges(g, start_id, end_id, &start_c, &start_size)) == NULL)
	{	printf("by edgeBetweenness\n");
		return -1;
	}
	if ((end_res = pam_reachEdges(g, end_id, start_id, &end_c, &end_size)) == NULL)
	{	printf("by edgeBetweenness\n");
		return -1;
	}
	
	if ((resSet = pam_reachNodesN(g, start_id)) == NULL)
	{	printf("by edgeBetweenness\n");
		return -1;
	}
	
	
	//ignore nodes that add zero to centrality
	/*int constraints[start_size+end_size+1];
	for (i=0; i<start_size; i++)
	{	constraints[i] = start_res[i].p_m.id;
		//printf("id: %d, dist: %d\n", start_res[i].p_m.id, start_res[i].p_m.distance);
	}
	for (i=start_size, j=0; i < start_size+end_size; i++, j++)
	{	constraints[i] = end_res[j].p_m.id;
		//printf("id: %d, dist: %d\n", end_res[j].p_m.id, end_res[j].p_m.distance);
	}
	constraints[start_size+end_size] = -1;
	set_constraints(resSet, constraints, 0);*/
	
	int dist, valid_paths, nonvalid, valid_paths2, nonvalid2, all_paths;
	pam_cell *cell;
	
	for (i=1; i<start_size; i++)
	{	cell = pam_lookup_cell(g, start_res[i].p_m.id);	
		pam_init_resSet(resSet, cell);
		
		while (pam_next(resSet, &res))
		{	
			for (j=1; j<end_size; j++)
			{	if (end_res[j].p_m.id == res.id)
					break;
			}
			if (j >= end_size)
				continue;
				
			dist = start_res[i].p_m.distance + end_res[j].p_m.distance - 1;
			//printf("j: %d, dist: %d, res.distance: %d start: %d, end: %d\n", j, dist, res.distance, start_res[i].p_m.distance, end_res[j].p_m.distance);
			if (dist <= res.distance)
			{	
				split_double(start_res[i].weight, &valid_paths, &nonvalid);
				split_double(end_res[j].weight, &valid_paths2, &nonvalid2);
			
				if ( dist < res.distance )
				{	centrality += ((double)valid_paths*valid_paths2)/(nonvalid*nonvalid2);
					//printf("dist < res.distance: %lf\n", ((double)valid_paths*valid_paths2)/(nonvalid*nonvalid2));
				}
				else if (dist == res.distance)
				{
					all_paths = (int)pam_reachNode2(g, start_res[i].p_m.id, end_res[j].p_m.id, 2);
					centrality += (double)valid_paths*valid_paths2/all_paths;
					//printf("IDS: %d-%d, dist == res.distance: %d\n", start_res[i].p_m.id, end_res[j].p_m.id, dist);
				}
			}
		}
	}
			
		
	pam_destroy_resultSet(resSet);
	free(end_res);
	free(start_res);
	
	//printf("start %d: %lf, end_%d: %lf\n", start_id, start_c, end_id, end_c);
	
	
	return (centrality - 1 + start_c + end_c);	
}
/*
set_2nd_offsets(pam_graph g, int i, int j, int *m, int *n) {
	if (j+1 >= g->hashtable[i]->occupied_cells) // end of bucket.
			{	*m = i+1; // 2nd node goes to next bucket
				*n = 0;
			}
			else
			{	*m = i;
				*n = j+1;
			}
}*/

double modularity(pam_graph *g)
{	int i, j; //offsets for first node
	int m, n; //offsets for second node
	int maxCC=0, numCC=0, offset, offset2;
	char  flag;
	unsigned char **table;
	double sum=0, are_neighbours, degree1, degree2;
	pam_lite_edge *edge;

	if ((table = (unsigned char**)create_tableOf_g(g, sizeof(unsigned char))) == NULL)
	{	printf("modularity: Cannot allocate memory for table\n");
		return INFINITY_REACH_NODE;
	}
	
	if (findCCs(g, &maxCC, &numCC, NULL, table))
	{	printf("by modularity\n");
		return INFINITY_REACH_NODE;
	}
	
	// for every pair of nodes
	// second node always proceeds first node in graph to avoid checking duplicates
	for (i=0; i < g->current_size; i++)
	{	for (j=0; j < g->hashtable[i]->occupied_cells; j++)
		{	flag = 0;
		
			if (j+1 >= g->hashtable[i]->occupied_cells) // end of bucket.
			{	m = i+1; // 2nd node goes to next bucket
				n = 0;
			}
			else
			{	m = i;
				n = j+1;
			}
			for (; m < g->current_size; m++)
			{	if (flag)
					n = 0; // set n to 0 except for 1st time
				else
					flag = 1; // 1st time
				for (; n < g->hashtable[m]->occupied_cells; n++)
				{	
					//1. nodes must belong in the same CC
					if (table[i][j] == table[m][n])
					{						
						//2. degrees of nodes
						degree1 = get_degree(g->hashtable[i]->my_cells[j]);
						degree2 = get_degree(g->hashtable[m]->my_cells[n]);
												
						//3. are they neighbours?
						are_neighbours = 0;						
						edge = g->hashtable[m]->my_cells[n].my_list[g->my_node_type].my_edges;
						while (edge != NULL)
						{	if (edge->ending_id == g->hashtable[i]->my_cells[j].my_node->id)
							{	are_neighbours = 1;
								break;
							}
							edge = edge->next_pam_edge;
						}
							
						sum += are_neighbours - degree1*degree2/(double)g->num_of_edges;
						
					}
					
				}
			}
		}
	}
	destroy_table(g, (void**)table);
	//printf("sum is %lf\n", sum);
	return (sum/(double)g->num_of_edges);
}

void *betweennness_all(void *arg) {
	char offsetI[11], offsetJ[11];
	pam_graph *g = (pam_graph*)arg;
	double betweenness;
	pam_result res;
	
	// consumer
	while (1)
	{
		//CRITICAL SECTION
			if (pthread_mutex_lock(mutex))
			{	perror("betweennness_all/mutex_lock");
				return;
			}
			
			while(edge_results_start == edge_results_end ) //result table is empty. wait.
			{	
				idle_counter++;
				if (idle_counter == NUMBER_OF_THREADS)
				{	
					pthread_cond_broadcast(&cond_finish);
				}
				pthread_cond_wait(&cond_empty, mutex);
				
				if (quit == 1)
					break;
				idle_counter--;
			}
		
			if (quit == 1)
			{	if (pthread_mutex_unlock(mutex))
				{	perror("betweennness_all/mutex_unlock");
					return;
				}
				break;
			}
			
			res = edge_results[edge_results_start];
			//printf("consumer awake. consuming no.%d: start-end: %d-%d\n", edge_results_start, res.distance, res.id);
			//edge_results_size--;
			edge_results_start = (edge_results_start + 1) % INITIAL_FRINGE_SIZE;
			
			pthread_cond_broadcast(&cond_full);
		
			if (pthread_mutex_unlock(mutex))
			{	perror("betweennness_all/mutex_unlock");
				return;
			}
		//END OF CRITICAL
	
		if ((betweenness = edgeBetweenness(g, res.distance, res.id)) == -1)
		{	printf ("by betweennness_all\n");
			return;
		}
	
	//CRITICAL SECTION
			if (pthread_mutex_lock(&mutex_max))
			{	perror("betweennness_all/mutex_lock");
				return;
			}
			
			if (betweenness > max_betweenness)
			{	max_betweenness = betweenness;
				max_start_id = res.distance;
				max_end_id = res.id;
			}
		
			if (pthread_mutex_unlock(&mutex_max))
			{	perror("betweennness_all/mutex_unlock");
				return;
			}
	//END OF CRITICAL
	
	}
	
	
}


int create_gn_Communities(pam_graph *g, gn_Communities *cmm)
{	int numofCCs=0, CCsize=0, i, j, id, *counters;
	unsigned char **belongsCC; 

	if ((belongsCC = (unsigned char**)create_tableOf_g(g, sizeof(unsigned char))) == NULL)
	{	printf("create_gn_community: Cannot allocate memory for belongs_table\n");
		return -1;
	}
	

	//get number of gn_Communities
	if (findCCs(g, &CCsize, &numofCCs, NULL, belongsCC) != 0)
	{	printf("by create gn_Communities\n");
		return -1;
	}
	cmm->size = numofCCs;

	if ((cmm->data = calloc(numofCCs, sizeof(gn_Community))) == NULL)
	{	printf("Cannot allocate memory for gn_Communities\n");
		return -1;
	}
	if ((counters = calloc(numofCCs, sizeof(int))) == NULL)
	{	printf("Cannot allocate memory for counters\n");
		return -1;
	}
	
	//find the size of each gn_Community
	for (i=0; i < g->current_size; i++)
		for (j=0; j < g->hashtable[i]->occupied_cells; j++)
			cmm->data[belongsCC[i][j] - 1].size++;
			
	for (i=0; i < numofCCs; i++)	
		if ((cmm->data[i].data = malloc(cmm->data[i].size * sizeof(int))) == NULL)
		{	printf("cannot allocate memory for size of each communtity\n");
			return -1;
		}
	
	//insert the id to each gn_Community
	int CC;
	for (i=0; i < g->current_size; i++)
		for (j=0; j < g->hashtable[i]->occupied_cells; j++)
		{	id = g->hashtable[i]->my_cells[j].my_node->id;
			
			CC = belongsCC[i][j] -1;
			cmm->data[CC].data[counters[CC]++] = id;
		}
		
	free(counters);
	destroy_table(g, (void**)belongsCC);
	
	return 0;
}

gn_Communities* Girvan_Newman(pam_graph *g, double limit_modularity) {
	int start_id, end_id, i, j, k, node_id, counter;
	pam_resultSet *resSet;
	global_g = g;
	double mdlrt = 0, previous_mdlrt = 0;
	pam_properties *prop;
	pam_edge *edge;
	
	init_global_variables();
	
	//find first non empty bucket to get a cell
	if ((i = empty_graph(g)) >= g->current_size)
	{	printf("Girvan-Newman: empty Graph\n");
		return NULL;
	}
	node_id = g->hashtable[i]->my_cells[0].my_node->id;
	
	
	//desmeush mnhmhs mutex
	if ((mutex = malloc(sizeof(pthread_mutex_t))) == NULL)
	{	printf("Girvan_Newman: Cannot allocate memory for mutex\n");
		return NULL;
	}
	if (pthread_mutex_init(mutex, NULL) != 0)
	{	perror("Girvan-Newman/mutex_destroy");
		return NULL;
	}
	
	pthread_t threads[NUMBER_OF_THREADS];
	
	// threads that find centrality of an edge (called in endless loop for all edges)
	for (i=0; i< NUMBER_OF_THREADS; i++)
		if (pthread_create(threads+i, NULL, betweennness_all, (void*)g))
			{	perror("Girvan-Newman: error while creating threads -");
				return NULL;
			}
	
	
	do
	{	counter = 0;
		//re-initiate in case recalculating betweenness is needed 
		previous_mdlrt = mdlrt;
		max_start_id = max_end_id = -1;
		max_betweenness = 0;
		edge_results_end = edge_results_start = 0;
	
		//Find edge with largest betweenness
		pam_result res;
		if ((resSet =  pam_reachNodesN(g, node_id)) == NULL)
		{	printf("by Girvan-Newman\n");
			return NULL;
		}
		set_constraints(resSet, NULL, GN_EDGES);
		resSet->my_fringe->visited_counter = 0;
		pam_cell *cell;
	
		for (i=0; i < g->current_size; i++)
		{	for (j=0; j < g->hashtable[i]->occupied_cells; j++)
			{	cell = g->hashtable[i]->my_cells + j;
		
				if (visited_node(cell->my_node->id, resSet->my_fringe->visited, resSet->my_fringe->visited_counter))
					continue;
					
				//epanarxikopoihsh resultSet
				resSet->my_fringe->the_fringe[0] = cell;
				resSet->my_fringe->the_fringe_counter = 1;
				resSet->my_fringe->temp_fringe_counter = 0;
				resSet->current_node = 0;

				
				if (pam_next(resSet, &res) == 0) //first call returns starting node
				{	printf("by Girvan_Newman\n");
					return NULL;
				}	
				
				
				while (pam_next(resSet, &res))
				{	//producer
	
					//CRITICAL SECTION
					if (pthread_mutex_lock(mutex))
					{	perror("Girvan_Newman/mutex_lock");
						break;
					}
				
					//printf("producer sleep???\n");
					while ((edge_results_end + 1) % INITIAL_FRINGE_SIZE == edge_results_start)
					{ 	//printf("YESS\n");
						pthread_cond_wait(&cond_full, mutex); 
				
					}
					counter++;
					//printf("producer awake. Producing no.%d: start-end: %d-%d\n", edge_results_end, res.distance, res.id);
			
					edge_results[edge_results_end] = res;
					//edge_results_size++;
					edge_results_end = (edge_results_end + 1) % INITIAL_FRINGE_SIZE;
			
					pthread_cond_broadcast(&cond_empty);
		
					if (pthread_mutex_unlock(mutex))
					{	perror("Girvan_Newman/mutex_unlock");
						break;
					}
					//END OF CRITICAL
	
					//printf("Start_node - end_node: %d - %d\n", res.distance, res.id);
				}
			}
		}
			
		
		pam_destroy_resultSet(resSet);
		
		if (pthread_mutex_lock(mutex))
		{	perror("Girvan_Newman/mutex_lock");
			break;
		}
	
		while(edge_results_end != edge_results_start)
		{	
			pthread_cond_wait(&cond_finish, mutex);
			
		}
		
		if (pthread_mutex_unlock(mutex))
		{	perror("Girvan_Newman/mutex_unlock");
			break;
		}
	
		if (max_start_id == -1 || max_end_id == -1)
		{	printf("Girvan-Newman, not existing edge: startID:%d endID: %d\n", max_start_id, max_end_id);
			return NULL;
		}
	
		// call twice to destroy both directions of edge with largest betweenness
		pam_destroy_edge(g, max_start_id, max_end_id, g->my_node_type);
		pam_destroy_edge(g, max_end_id, max_start_id, g->my_node_type);
	
		mdlrt = modularity(g);
		//printf("edges: %d check modul: %lf\n", counter, mdlrt);
		
		//check if we surpassed the modularity limit or if we reached the highest modularity
	}	while (limit_modularity > mdlrt  && previous_mdlrt < mdlrt);
	
	if (previous_mdlrt >= mdlrt) 
	{	//reached highest modularity. re-insert edge that was destroyed
		prop = setPkP_Properties();
		edge = pam_createEdge(max_start_id, max_end_id, prop, person, person_knows_person);
		pam_insertEdge(g, edge);
		//both directions
		prop = setPkP_Properties();
		edge = pam_createEdge(max_end_id, max_start_id, prop, person, person_knows_person);
		pam_insertEdge(g, edge);
	}
	
	//printf("hello about to end\n");
	quit = 1;
	pthread_cond_broadcast(&cond_empty);
	
	for (i=0; i <  NUMBER_OF_THREADS; i++)
		if(pthread_join(threads[i], NULL))
		{	perror("Girvan-Newman: error while joining threads -");
			return NULL;
		}
	
	int num;
	if ((num = pthread_mutex_destroy(mutex)) != 0)
	{	printf("%d code\n", num);
		perror("Girvan-Newman/mutex_destroy(near end)");
		return NULL;
	}
	
	free(mutex);
	
	//create the communites
	gn_Communities *cmm;
	if ((cmm = malloc(sizeof(gn_Communities))) == NULL)
	{	printf("Girvan_newman: Cannot allocate memory for gn_communities\n");
		return NULL;
	}
	
	if (create_gn_Communities(g, cmm) != 0)
	{	printf("by Girvan-newman\n");
		return NULL;
	}
		
	return cmm;
}