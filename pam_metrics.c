//pam_metrics.c

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <float.h> 
#include "pam_utilities.h"
#include "pam_graph.h"
#include "defines.h"
#include "pam_metrics.h"

//1. Degree Distribution
void degreeDistribution(pam_graph *g) {
	int *count_of_degree, degree, i, j, pid;
	pam_lite_edge *edge_ptr;
	FILE *data_file;
	if ((count_of_degree = calloc(g->num_of_nodes, sizeof(int))) == NULL)
	{	printf("degreeDistribution: Cannot allocate memory for counters\n");
		return;
	}
	for (i=0; i < g->current_size; i++)
	{	for (j=0; j < g->hashtable[i]->occupied_cells; j++)
		{	degree = get_degree(g->hashtable[i]->my_cells[j]);
			count_of_degree[degree]++;
		}
	}
	//ftiakse ta dedomena ths sunarthshs gia to gnuplot
	if ((data_file = fopen("input.dat", "w")) == NULL)
	{	printf("degreeDistribution: Cannot create data file\n");
		return;
	}
	fprintf(data_file, "%d %f\n\n", 0, ((double)count_of_degree[0])/g->num_of_nodes);
	for (i=1; i < g->num_of_nodes; i++)
	{	//count_of_degree[i] += count_of_degree[i-1];
		//printf("%d %f\n\n", g->num_of_nodes, ((double)count_of_degree[i])/g->num_of_nodes);
		fprintf(data_file, "%d %f\n\n", i, ((double)count_of_degree[i])/g->num_of_nodes);
		//count_of_degree[i] += count_of_degree[i-1];
	}
	fclose(data_file);
	
	//ftiakse to arxeio me tis entoles tou gnuplot
	if ((data_file = fopen("commands.gp", "w")) == NULL)
	{	printf("degreeDistribution: Cannot create command file\n");
		return;
	}
	fprintf(data_file, "set terminal dumb\nplot \"input.dat\"\n");
	fclose(data_file);
	
	if ((pid = fork()) == -1)
	{	printf("degreeDistribution: Cannot create child process\n");
		return;
	}
	if (pid == 0)
	{	execlp("gnuplot", "gnuplot", "commands.gp", NULL);
	}
	free(count_of_degree);
	return;
}


//2. Diameter
int diameter(pam_graph *g){
	int max_distance=0,current_max_distance=0,my_id=0,i,j=0;
	pam_bucket **hasht;
	pam_resultSet *my_resultSet;
	pam_result res;
	hasht=g->hashtable; 
	//find a non-empty bucket
	if ((i = empty_graph(g)) >= g->current_size)
	{	printf("diameter: empty Graph\n");
		return -1;
	}
			
	my_resultSet = pam_reachNodesN(g,hasht[i]->my_cells[j].my_node->id);
	
	while(i<g->current_size){
		while(j<hasht[i]->occupied_cells){
	
				//epanarxikopoihsh resultSet
				my_resultSet->my_fringe->the_fringe[0] = &(hasht[i]->my_cells[j]);
				my_resultSet->my_fringe->the_fringe_counter = 1;
				my_resultSet->my_fringe->temp_fringe_counter = 0;
				my_resultSet->my_fringe->visited[0] = hasht[i]->my_cells[j].my_node->id;
				my_resultSet->my_fringe->visited_counter = 1;
				my_resultSet->dist = my_resultSet->current_node = 0;
				
				while(pam_next(my_resultSet,&res)!=false){
				    current_max_distance=res.distance;
				    if(max_distance<current_max_distance){
				        max_distance=current_max_distance;
					}
				}
				j++;
				
		}
		i++;
		j=0;
	}
	pam_destroy_resultSet(my_resultSet);
	return max_distance;
}


//3. Average Path Length
double averagePathLength(pam_graph *g){
	int distance_sum = 0;
	int min_distance = 0;
	int i, j=0;
	pam_resultSet *my_resultSet;
    pam_bucket **hasht;
	pam_result res;
    hasht=g->hashtable;
	
	//find a non-empty bucket
	if ((i = empty_graph(g)) >= g->current_size)
	{	printf("averagePathlength: empty Graph\n");
		return -1;
	}
	my_resultSet = pam_reachNodesN(g,hasht[i]->my_cells[j].my_node->id);
	
    while(i<g->current_size){
		while(j<hasht[i]->occupied_cells){
			
				//epanarxikopoihsh resultSet
				my_resultSet->my_fringe->the_fringe[0] = &(hasht[i]->my_cells[j]);
				my_resultSet->my_fringe->the_fringe_counter = 1;
				my_resultSet->my_fringe->temp_fringe_counter = 0;
				my_resultSet->my_fringe->visited[0] = hasht[i]->my_cells[j].my_node->id;
				my_resultSet->my_fringe->visited_counter = 1;
				my_resultSet->dist = my_resultSet->current_node = 0;
			
				while(pam_next(my_resultSet,&res)!=false){
				    min_distance=res.distance;
					distance_sum=distance_sum+min_distance;
					}
				j++;
			
			}
		i++;
		j=0;
		}
	pam_destroy_resultSet(my_resultSet);
    return (1.0/(double)((g->num_of_nodes)*(g->num_of_nodes -1 )))*distance_sum;
	}


//4. Number Of CCs
int numberOfCCs(pam_graph *g) {
	int maxCCsize=0, numofCCs=0;
	findCCs(g, &maxCCsize, &numofCCs, NULL, NULL);
	return numofCCs;
}


//5. Max CC
int maxCC(pam_graph* g) {
	int maxCCsize=0, numofCCs=0;
	findCCs(g, &maxCCsize, &numofCCs, NULL, NULL);
	return maxCCsize;
}


//6. Density
double density(pam_graph *g){
	int edges,nodes;
	edges = g->num_of_edges;
	nodes = g->num_of_nodes;
	return (double)(2 * edges) / ((double)nodes*(nodes-1));  //check this point
	}

	
//7. Centrality
//A. Closeness Centrality
double pam_closenessCentrality_backup(pam_node* n, pam_graph* g){
	int i, current_value;
	double total_sum = 0, cc, ccn;
	pam_result* temp_pair = malloc(sizeof(pam_result));
	
	//0. Elegxos dedomenwn
	if(n==NULL || g==NULL){
		printf("closenessCentrality: Invalid input.\n");
		return -1;
		}
	if(g->num_of_nodes<=1){
		return 0;
		}
	
	//1. Vres to synektiko komvo pou periexei ton komvo n.
	pam_resultSet* my_resultSet = pam_reachNodesN(g, n->id);
	if(my_resultSet==NULL){
		printf("closenessCentrality: Unable to find Node.\n");
		return -1;
		}
	
	//Epektash toy grafou mexri na prospelasoume olous tou komvous
	while(pam_next(my_resultSet, temp_pair)){
		current_value = temp_pair->distance;
		if(current_value!=0 && current_value!=-1){
			total_sum += current_value;
			//printf("current_node: %d \ttotal_sum: %f \tcurrent_value: %d\n",temp_pair->id, total_sum, current_value);
			}
		}
	
	//printf("g->gr->num_of_nodes: %d\n", g->num_of_nodes);
	
	cc = (double)1/total_sum;
	ccn = cc/(double)(g->num_of_nodes - 1);
	
	//printf("cc: %f \t ccn: %f\tnum_of_nodes - 1: %d\ttotal_sum: %f\n", cc, ccn, g->num_of_nodes - 1, total_sum);

	free(temp_pair);
	pam_destroy_resultSet(my_resultSet);
	return ccn;
	}

//A. Closeness Centrality
double pam_closenessCentrality(pam_node* n, pam_graph* g){
	int i, current_value;
	double cc = 0, ccn;
	pam_result* temp_pair = malloc(sizeof(pam_result));
	
	//0. Elegxos dedomenwn
	if(n==NULL || g==NULL){
		printf("closenessCentrality: Invalid input.\n");
		return -1;
		}
	if(g->num_of_nodes<=1){
		return 0;
		}
	
	//1. Vres to synektiko komvo pou periexei ton komvo n.
	pam_resultSet* my_resultSet = pam_reachNodesN(g, n->id);
	if(my_resultSet==NULL){
		printf("closenessCentrality: Unable to find Node.\n");
		return -1;
		}
	
	//Epektash toy grafou mexri na prospelasoume olous tou komvous
	while(pam_next(my_resultSet, temp_pair)){
		current_value = temp_pair->distance;
		if(current_value > 0){
			cc += (double)1/(double)current_value;
			//printf("current_node: %d \tcurrent_value: %d\tcc: %f\n",temp_pair->id, current_value, cc);
			}
		}
	
	//printf("g->gr->num_of_nodes: %d\n", g->num_of_nodes);
	
	ccn = cc/(double)(g->num_of_nodes - 1);
	
	free(temp_pair);
	pam_destroy_resultSet(my_resultSet);
	return ccn;
	}


//B. Betweenness Centrality
double pam_betweennessCentrality(pam_node* n, pam_graph* g){
	/*
	To Betweeness Centrality einai mia xrhsimh alla dapanhrh metrikh gia to grafo.
	Gia na ypologistei loipon xrhsimopoih8hke o algori8mos tou Ulrik Brandes, opws anaferetai kai sto readme.
	
	Logw ths fyshs tou algori8mou omws to Betweeness Centrality ypologizetai gia olo to grafo se ka8e klhsh tou algori8mou.
	Etsi apofasisame na krat;ame ta apotelesmata se enan pinaka gia na mhn xreiazetai 3ana ypologimos,
	mias kai o xwros pou katalamvanoun den einai idiaitera megalos.
	
	*/
	
	int num_of_nodes = g->num_of_nodes;
	int i, j, k;
	int temp_size;
	int current_distance, neighbour_distance;
	pam_list* my_node_list;	//Xrhsimopoieitai gia na apo8hkeyontai oloi oi komvoi
	node_entry*  s, * w, * u, * last_u;
	pam_lite_edge* current_edge;
	pam_cb_result* betweeness_centrality_results;
	pam_stack* S;
	pam_queue* Q;
	
	
	//A. Elegxos dedomenwn
	if(n==NULL || g==NULL){
		printf("pam_betweennessCentrality: Invalid input\n");
		return -1;
		}
		
	//B. An den exei ypologistei hdh to Betweeness Centrality, ypologise to
	if(g->betweeness_centrality_results == NULL){
		//1. Desmevsh xwrou	
		if( (my_node_list = create_node_list(g)) == NULL){
			printf("pam_betweennessCentrality: Error allocating space\n");
			return -1;
			}
		if( (betweeness_centrality_results = malloc(num_of_nodes * sizeof(pam_cb_result))) == NULL){
			printf("pam_betweennessCentrality: Error allocating space\n");
			return -1;
			}
		
		//2. Gia ka8e Node sto Grafo
		for(i=0; i<num_of_nodes; i++){
			//Arxikopoihsh
			S = pam_create_stack();
			Q = pam_create_queue();
			
			//Ekkinhsh diadikasias
			s = get_node_with_offset(i, my_node_list);
			s->sigma = 1;
			s->d = 0;
			pam_push_to_queue(Q, s);
			//Oso yparxoun shmantikoi komvoi
			while(get_queue_size(Q)>0){
				u = pam_pop_from_queue(Q);
				pam_push_to_stack(S, u);
				
				current_edge = u->my_element->my_list[g->my_node_type].my_edges;
				current_distance = u->d;
				//printf("Node id: %d\tNode id: %d\tcurrent sist: %d\n", s->my_element->my_node->id, u->my_element->my_node->id, current_distance);
				
				//Gia olous toys geitones
				while(current_edge != NULL){
					w = get_node_with_id(current_edge->ending_id, my_node_list);
					neighbour_distance = w->d;
					//printf("Neighbour id: %d (real: %d)\tDistance: %d\n", current_edge->ending_id, w->my_element->my_node->id, neighbour_distance);
					//printf("Node id: %d\t neighbour_distance: %d\n", w->my_element->my_node->id, neighbour_distance);
				
					//An einai kainourios komvos
					if(neighbour_distance<0){
						pam_push_to_queue(Q, w);
						w->d = current_distance+1;
						neighbour_distance = current_distance+1;
						//printf("NEW NODE! New distance: %d\n", w->d);
						}
					
					//An to kontinotero monopati einai mesw tou current Node
					if(neighbour_distance == current_distance+1){
						w->sigma = w->sigma + u->sigma;
						pam_append_to_list(w->P, u);
						//pam_push_to_list(P, u);
						//printf("SHORTEST PATH! Distance: %d\tSigma %d\n", w->d, w->sigma);
						//printf("w->delta: %f\tw->sigma: %d\tw->d: %d\t", w->delta, w->sigma, w->d);
						//printf("w->delta: %f\tw->sigma: %d\tw->d: %d\n", u->delta, u->sigma, u->d);
						}
					
					current_edge = current_edge->next_pam_edge;
					}
				}
			//printf("Stack size: %d\n", get_stack_size(S));
			//printf("List size: %d\n", get_list_size(P));
			while(get_stack_size(S)>0){
				w = pam_pop_from_stack(S);
				if(w->sigma != 0){
					last_u = NULL;
					temp_size = get_list_size(w->P);
					for(j=0; j<temp_size; j++){
						u = pam_get_from_list(w->P, j);
						if(u != last_u){
							u->delta = 
								u->delta + 
								((double)(u->sigma) / (double)(w->sigma) * 
								(1.0 + w->delta));
							  
							last_u = u;
							}
						}
					if(w!=s){
						w->cb = w->cb + w->delta;
						}
					}
				}
			
			//Termatismos
			pam_destroy_stack(S);
			pam_destroy_queue(Q);
			
			node_list_reset(my_node_list);	//Kaleitai gia na mhdenisei ta delta
			}
		
		
		//3. Apouhkeyse sto grafhma ton deikth ston pinaka kai arxikopoihse ton
		for(i=0; i<num_of_nodes; i++){
			s = get_node_with_offset(i, my_node_list);
			betweeness_centrality_results[i].id = s->my_element->my_node->id;
			//printf("Result for id: %d\t%f\n", s->my_element->my_node->id, s->cb);
			betweeness_centrality_results[i].cb = ((double) s->cb ) /
												  ((double)((num_of_nodes-2) * (num_of_nodes - 1)));//Normalized
			}
		g->betweeness_centrality_results = betweeness_centrality_results;
		
		pam_destroy_list(my_node_list);
		}
		
		
	//4. Eyresh kai epistrofh toy apo telesmatos
	for(i=0; i<num_of_nodes; i++){
		if(g->betweeness_centrality_results[i].id == n->id){
			return g->betweeness_centrality_results[i].cb;
			}
		}
	
	printf("pam_betweennessCentrality: Unable to find result.\n");
	return -1;
	
	}
