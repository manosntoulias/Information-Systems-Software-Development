//cpm_v2.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "pam_graph.h"
#include "pam_utilities.h"
#include "defines.h"
#include "community_algorithms.h"

#define MAX_NUM_OF_THREADS 100



//UNIVERSAL METAVLHTES
//Xrhsimopoiountai gia na empodisoun thn taytoxronh prosvash se:
pthread_mutex_t access_to_clique_graph;	//Grapho me cliques
pthread_mutex_t access_to_id_count;	//Metrhth trexontos id (pou pairnoun oi cliques)
pthread_mutex_t modify_num_of_threads;	//Trexon ari8mos threads

//Empodizoun thn enar3h threads, an exoume to max plh8os
pthread_mutex_t allow_new_threads;
pthread_mutex_t thread_finish;
pthread_mutex_t thread_finish_control;
pthread_cond_t max_num_of_threads_reached;

int clique_size;	//Mege8os clique
int current_clique_id;



//STRUCTS
//STRUCTS: Clique
typedef struct clique{
	pam_cell** clique_cells;
	} clique;

typedef struct clique_data{
	pam_graph* initial_graph;
	pam_graph* clique_graph;
	int* running_threads_counter;
	pam_cell* current_cell;
	} clique_data;

typedef struct control_data{
	int num_of_nodes;
	int* running_threads;
	int* total_threads;
	} control_data;

	
	
//UTILITIES
//Clique management
clique* create_clique(){
	clique* my_clique = malloc(sizeof(clique));
	
	if(my_clique == NULL){
		printf("create_clique: Error: Unable to allocate memory.\n");
		return NULL;
		}
		
	my_clique->clique_cells = calloc(clique_size, sizeof(pam_cell*));
	return my_clique;
	}
	
void destroy_clique(clique* my_clique){
	
	if(my_clique != NULL){
		free(my_clique->clique_cells);
		free(my_clique);
		}
		
	else{
		printf("destroy_clique: Error: Invalid input.\n");
		}
		
	}

	
//CCs
int find_CC_community(int *i, int *j, pam_graph* clique_graph, pam_resultSet *resSet, int *constraints, Communities* my_communities){
	//vriskei ena grafhma gia kathe klish
	//epistrefei  0 otan den uparxei allo grafhma
	pam_result res;
	char start_bfs=0, no_constraint;
	int k, new_size, clique_size, clique_counter, communities_counter;
	int* clique_ids, ** current_community = NULL;
	pam_cell *cell_ptr;
	Community* my_community;
	
	resSet->dist = 0;
	
	
	//printf("find_CC_community: Creating Community.\n");
	//1. Dhmiourgia neas Community
	if( (my_community = malloc(sizeof(Community))) == NULL){
		printf("find_CC_community: Error: Unable to allocate memory.\n");
		return 0;
		}
	
	
	//2. Pros8hkh twn kainouriwn cliques
	clique_counter = 0;
	clique_size = my_communities->clique_size;
	while( pam_next(resSet, &res) != false){
		clique_counter++;
		//printf("find_CC_community: Current Clique %d\n", clique_counter);
		
		//Desmeysh mnhmhs gia thn kainouria clique
		current_community = realloc(current_community, clique_counter * sizeof(int*));
		//printf("find_CC_community: Realloced\tclique_size: %d\n", clique_size);
		if( (clique_ids = malloc(clique_size * sizeof(int))) == NULL){
			printf("find_CC_community: Error: Unable to allocate memory.\n");
			return 0;
			}
		
		//Apo8hkeysh kainourias clique
		//printf("find_CC_community: Storing\n");
		cell_ptr = pam_lookup_cell(clique_graph, res.id);
		for(k=0; k<clique_size; k++){
			clique_ids[k] = cell_ptr->my_node->id;
			}
		current_community[clique_counter - 1] = clique_ids;
		//printf("find_CC_community: Stored\n\n");
		}
	
	//printf("find_CC_community: Finished!\n");
	
	//3. Apo8hkeysh Community
	my_community->data = current_community;
	my_community->size = clique_counter;
	communities_counter = (++my_communities->size);
	if( (my_communities->data = realloc(my_communities->data, communities_counter * sizeof(Community*))) == NULL){
		printf("find_CC_community: Error: Unable to allocate memory.\n");
		return 0;
		}
	//Apo8hkeysh synektikou grafou sth domh Communities
	(my_communities->size)++;
	my_communities->data[communities_counter - 1] = my_community;
	
	
	
	// vres enan komvo pou den exoume episkeftei psaxnontas seiriaka olous tous komvous
	for (; *i < resSet->my_graph->current_size; (*i)++){
		for (; *j < resSet->my_graph->hashtable[*i]->occupied_cells; (*j)++){
			cell_ptr = &(resSet->my_graph->hashtable[*i]->my_cells[*j]);
			no_constraint=0;
			if (constraints == NULL){
				no_constraint=1;
				}
			else
				for (k=0; constraints[k] != -1; k++)
					if (constraints[k] == cell_ptr->my_node->id){
						no_constraint = 1;
						}
			
			if (no_constraint){
				if (!visited_node(cell_ptr->my_node->id, resSet->my_fringe->visited, resSet->my_fringe->visited_counter)){
					//printf("22: i: %d, j: %d, node_my_node->id: %d\n", *i, *j, cell_ptr->my_node->id);
					start_bfs = 1;
					break;
					}
				}
			}
		
		if (start_bfs){
			break;
			}
		
		*j = 0;
		}
	//episkeuthkame olous tous kombous tou graphou. Telos anazhthshs
	if (*i == resSet->my_graph->current_size){
		return 0;
		}
	
	// epanazrxikopoihsh metablhtwn gia epomeno grafhma
	// kratame ta visited kai to distance
	
	resSet->my_fringe->the_fringe[0] = cell_ptr;
	resSet->my_fringe->the_fringe_counter = 1;
	resSet->my_fringe->temp_fringe_counter = 0;
	if (resSet->my_fringe->visited_counter == resSet->my_fringe->visited_size){
		new_size = resSet->my_fringe->visited_size * INITIAL_FRINGE_SIZE;
		if( (resSet->my_fringe->visited = realloc(resSet->my_fringe->visited, new_size * sizeof(int))) == NULL){
			printf("find_CC_community: Unable to reallocate memory for visited\n");
			return 0;
			}
		resSet->my_fringe->visited_size = new_size;
		}
	
	resSet->my_fringe->visited[resSet->my_fringe->visited_counter] = cell_ptr->my_node->id;
	
	//resSet->dist = 0;
	resSet->current_node = 0;
	
	//vrhkame grafhma alla uparxoun kai alloi komvoi pros eksereunish
	return 1;
	}

Communities* findCommunities(pam_graph* clique_graph){
	pam_resultSet *resSet;
	int i, j;
	int id, graph_size;
	int *constraints;
	int last_CC;
	Communities* my_communities;
	
	
	printf("findCommunities: 0. Elegxos dedomenwn.\n");
	//0. Elegxos dedomenwn
	if(clique_graph==NULL){
		printf("findCommunities: Invalid input.\n");
		return NULL;
		}
	
	
	printf("findCommunities: 1. Desmeysh mnhmhs kai arxikopoihsh\n");
	//1. Desmeysh mnhmhs kai arxikopoihsh
	graph_size = clique_graph->current_size;
	if( (my_communities = malloc(sizeof(Communities))) == NULL){
		printf("findCommunities: Error allocating memory.\n");
		return NULL;
		}
	my_communities->size = 0;
	my_communities->clique_size = clique_size;
	my_communities->data = NULL;
	
	//2. Vres to prwto mh keno bucket
	for(i=0; i<graph_size; i++){
		if(clique_graph->hashtable[i]->occupied_cells != 0){
			break;
			}
		}
	
	if (i == graph_size){
		printf("findCommunities: No communities found\n");
		return my_communities;
		}
	
	
	printf("findCommunities: 2. Vres ola ta CCs\n");
	//2. Vres ola ta CCs
	j = 0;
	id = clique_graph->hashtable[i]->my_cells[j].my_node->id;
	resSet = pam_reachNodesN(clique_graph, id);
	//set_constraints(resSet, constraints, 0);
	
	last_CC = 1;
	while(last_CC != 0){
		printf("%d", last_CC);
		last_CC = find_CC_community(&i, &j, clique_graph, resSet, NULL, my_communities);
		resSet->my_fringe->visited_counter++;
			printf("findCommunities: Community %d\n", resSet->my_fringe->visited_counter);
		}
		
	printf("\nfindCommunities: Finishing\n");
	pam_destroy_resultSet(resSet);
	return my_communities;
	}

//Threads utilities
void recursive_expand_clique(int current_level, pam_cell* my_cell, clique* my_clique, pam_graph* initial_graph, pam_graph* clique_graph){
	pam_node* my_node;
	pam_cell* neighbour_cell;
	pam_lite_edge* next_edge;
	pam_properties* my_properties;
	int i;
	int current_id;
	
	//printf("recursive_expand_clique: current_level: %d\n", current_level);
	
	//A. AN H CLIQUE FTASEI SE AYTO TO MEGE8OS, EINAI KAINOURIA KAI MONADIKH
	//Synepws prosti8etai sto grapho
	if(current_level == clique_size - 1){
		
		//Dhmiourgia Node
		my_properties = pam_createProperties(clique_size);
		if(my_properties == NULL){
			printf("recursive_expand_clique: Error: Unable to allocate memory.\n");
			return;
			}
		
		//printf("New clique!\n");
		for(i=0; i<clique_size; i++){
			//printf("%p Adding to clique: %d\n", my_clique, my_clique->clique_cells[i]->my_node->id);
			current_id = my_clique->clique_cells[i]->my_node->id;
			pam_setIntegerProperty(current_id, i, my_properties);
			}
			
		//Ekxwrhsh monadikou id
		pthread_mutex_lock(&access_to_id_count);
		current_id = current_clique_id;
		current_clique_id++;
		pthread_mutex_unlock(&access_to_id_count);
		
		my_node = pam_createNode(current_id, my_properties);
		if(my_node == NULL){
			printf("recursive_expand_clique: Error: Unable to allocate memory.\n");
			return;
			}
		
		//Pros8hkh Node sto graph
		pthread_mutex_lock(&access_to_clique_graph);
		pam_insertNode(clique_graph, my_node);
		pthread_mutex_unlock(&access_to_clique_graph);
		return;
		}
	
	//B. H CLIQUE DEN EXEI OLOKLHRW8EI
	current_level++;
		
	next_edge = my_cell->my_list[initial_graph->my_node_type].my_edges;
	while(next_edge != NULL){
		neighbour_cell = pam_lookup_cell(initial_graph, next_edge->ending_id);
		
		//Elegxos gia to an prepei na proste8ei o komvos
		//Ta melh ths clique eisagontai kata ay3onta id
		if(my_clique->clique_cells[current_level-1]->my_node->id < neighbour_cell->my_node->id){
			//Anadromikh klhsh
			my_clique->clique_cells[current_level] = neighbour_cell;
			recursive_expand_clique(current_level, neighbour_cell, my_clique, initial_graph, clique_graph);
			}
		
		next_edge = next_edge->next_pam_edge;
		}
	}

//Level 3: Threads
void* control_threads(void* args){
	control_data* my_data = (control_data*)args;
	int i;
	int num_of_nodes = my_data->num_of_nodes;
	int* running_threads = my_data->running_threads;
	int* total_threads = my_data->total_threads;
	
	for(i=0; i<num_of_nodes; i++){
		pthread_mutex_lock(&thread_finish_control);
		
		pthread_mutex_lock(&modify_num_of_threads);
		(*running_threads)--;
		//An mporei na 3ekinhsei kapoio mplokarismeno thread, 3ekinhse to
		if((*running_threads) == MAX_NUM_OF_THREADS - 1){
			pthread_cond_signal(&max_num_of_threads_reached);
			}
		pthread_mutex_unlock(&modify_num_of_threads);
		
		pthread_mutex_unlock(&thread_finish);
		}
	
	pthread_exit(NULL);
	}

void* find_individuals_cliques(void* args){
	clique_data* my_data = (clique_data*)args;
	
	pam_graph* initial_graph = my_data->initial_graph;
	pam_graph* clique_graph = my_data->clique_graph;
	pam_cell* my_cell = my_data->current_cell;
	int* running_threads_counter = my_data->running_threads_counter;
	
	clique* my_clique = create_clique();;
	int initial_level = 0;	//Xrhsimopoieitai gia na apo8hkeysei to trexon mege8os ths klikas
	
	
	//Anadromikh pros8hkh olwn twn klikwn
	my_clique->clique_cells[0] = my_cell;	//Topo8eteitai to prwto cell, wste na leitourgei swsta h parakatw synarthsh
	recursive_expand_clique(initial_level, my_cell, my_clique, initial_graph, clique_graph);
	
	//Diagrafh ths clique
	destroy_clique(my_clique);
	
	//printf("thread: cell id %d\n", my_cell->my_node->id);
	
	//Asfalhs termatismos tou thread
	pthread_mutex_lock(&modify_num_of_threads);
	(*running_threads_counter)--;
	
	//An mporei na 3ekinhsei kapoio mplokarismeno thread, 3ekinhse to
	if((*running_threads_counter) == MAX_NUM_OF_THREADS - 1){
		pthread_cond_signal(&max_num_of_threads_reached);
		}
	pthread_mutex_unlock(&modify_num_of_threads);
	
	pthread_exit(NULL);
	}
	
void* connect_cliques(void* args){
	clique_data* my_data = (clique_data*)args;
	
	pam_graph* clique_graph = my_data->clique_graph;
	pam_cell* my_cell = my_data->current_cell;
	pam_cell* current_cell;
	pam_properties* my_properities, * current_properties, * temp_properties;
	pam_edge*  temp_edge;
	int graph_size;
	int common_ids_counter;
	int starting_id, ending_id;
	int i, j;
	
	
	cell_map* my_cell_map = create_cell_map();
	graph_size = clique_graph->num_of_nodes;
	starting_id =  my_cell->my_node->id;
	my_properities = my_cell->my_node->my_properties;
	if(my_cell_map == NULL){
		printf("connect_cliques: Error: Error allocating memory.\n");
		
		//Asfalhs termatismos tou thread
		pthread_mutex_lock(&thread_finish);
		pthread_mutex_unlock(&thread_finish_control);
		
		pthread_exit(NULL);
		}
	
	//Elegxos olwn twn keliwn, an synoreyoun me to trexon
	for(i=0; i<graph_size; i++){
		current_cell = get_next_cell(my_cell_map, clique_graph);
		current_properties = current_cell->my_node->my_properties;
		
		common_ids_counter = 0;
		for(j=0; j<clique_size; j++){
			if(	*(int*)(my_properities->my_properties[j]) == 
				*(int*)(current_properties->my_properties[j])){
				common_ids_counter++;
				}
			}
		
		//An exoun k-1 koinous, pros8hkh sto grafo
		if(common_ids_counter = clique_size - 1){
			
			ending_id = current_cell->my_node->id;
			temp_properties = pam_createProperties(0);
			temp_edge = pam_createEdge(starting_id, ending_id, temp_properties, hypernode, hyperedge);
			
			//Asfalhs pros8hkh sto grafo
			pthread_mutex_lock(&access_to_clique_graph);
			pam_insertEdge(clique_graph, temp_edge);
			pthread_mutex_unlock(&access_to_clique_graph);
			}
		}
	
	//Asfalhs termatismos tou thread
	pthread_mutex_lock(&thread_finish);
	pthread_mutex_unlock(&thread_finish_control);
	
	pthread_exit(NULL);
	}

	

//ZHTOUMENOS ALGORI8MOS
Communities* cliquePercolationMethod(int k, pam_graph* my_graph){
	//Threads' management vars
	int running_threads;
	int total_threads;
	int rc;
	pthread_t control_thread;
	pthread_t* thread_array;
	pthread_attr_t attr;
	
	//Threads' algorithm vars
	cell_map* my_cell_map = create_cell_map();
	control_data* my_control_data;
	clique_data* my_data;
	
	//My vars
	Communities* my_communities;
	pam_graph* clique_graph;
	int* running_threads_counter;
	pam_cell* current_cell;
	int i, j;
	int num_of_nodes;
	
	
	
	//0. Elgxos dedomenwn kai esmeysh mnhmhs
	if(k<=0 || my_graph == NULL){
		printf("cliquePercolationMethod: Error: Invalid input.\n");
		return NULL;
		}
	
	//Ry8mish threads
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	pthread_mutex_init(&access_to_clique_graph, NULL);
	pthread_mutex_init(&access_to_id_count, NULL);
	pthread_mutex_init(&modify_num_of_threads, NULL);
	pthread_mutex_init(&allow_new_threads, NULL);
	pthread_mutex_init(&thread_finish, NULL);
	pthread_mutex_init(&thread_finish_control, NULL);
	
	pthread_cond_init (&max_num_of_threads_reached, NULL);
	
	
	//A. DHMIOURGIA GRAFOU ME NODES TA CLIQUES
	//1. Desmeysh mnhmhs
	clique_size = k;
	running_threads = 0;
	total_threads = 0;
	num_of_nodes = my_graph->num_of_nodes;
	my_cell_map = create_cell_map();
	thread_array = malloc(num_of_nodes * sizeof(pthread_t));
	
	my_control_data = malloc(sizeof(control_data));
	my_data = malloc(num_of_nodes * sizeof(clique_data));
	clique_graph = pam_createGraph(DEFAULT_HASHTABLE_SIZE, DEFAULT_NUM_OF_CELLS, hypernode);
	
	if(	my_cell_map==NULL || 
		thread_array == NULL || 
		my_data == NULL ||
		clique_graph == NULL ){
		printf("cliquePercolationMethod: Error: Unable to allocate memory.\n");
		return NULL;
		}
	
	
	//2. Vres tis cliques pou symmetexei ka8e Node toy grafou
	//	Dhmiourghse thread gia ka8e Node tou grafou
	for(i=0; i<num_of_nodes; i++){
		//printf("cliquePercolationMethod: Starting thread %d\n", i);
		//Fortwsh toy trexwn cell sta orismata
		my_data[i].initial_graph = my_graph;
		my_data[i].clique_graph = clique_graph;
		my_data[i].running_threads_counter = &running_threads;
		my_data[i].current_cell = get_next_cell(my_cell_map, my_graph);
		
		//Ekkinhsh thread
		//rc = pthread_create(&thread_array[i], &attr, &find_individuals_cliques, (void*)(&my_data[i]));
		rc = pthread_create(&thread_array[i], NULL, &find_individuals_cliques, (void*)(&my_data[i]));
		if (rc){
			printf("cliquePercolationMethod: Error: return code from pthread_create() is %d\n", rc);
			return NULL;
			}
		pthread_mutex_lock(&modify_num_of_threads);
		running_threads++;
		total_threads++;
		pthread_mutex_unlock(&modify_num_of_threads);
		
		//Elegxos plhthous trexontwn threads
		while(running_threads == MAX_NUM_OF_THREADS){
			pthread_cond_wait(&max_num_of_threads_reached, &allow_new_threads);
			}
		}
	
	
	//3. Perimene ola threads na oloklhrwthoun
	for(i=0; i<num_of_nodes; i++){
		pthread_join(thread_array[i], NULL);
		}
	
	//pam_print_graph(clique_graph);
	//pam_print_graph(my_graph);
	printf("cliquePercolationMethod: Successfully added Nodes to Graph.\n");
	
	
	//B. PROS8HKH TWN EDGES (OTAN YPARXOUN K-1 KOINOI KOMVOI)
	num_of_nodes = clique_graph->num_of_nodes;
	running_threads = 0;	//Prepei na einai etsi ki alliws
	total_threads = 0;
	
	
	
	//1. Epanadesmeysh mnhmhs
	destroy_cell_map(my_cell_map);
	my_cell_map = create_cell_map();
	free(thread_array);
	thread_array = malloc(num_of_nodes * sizeof(pthread_t));
	free(my_data);
	my_data = malloc(num_of_nodes * sizeof(clique_data));
	total_threads = 0;
	
	//Ry8mish threads
	pthread_attr_destroy(&attr);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	//2. Sygkrine ola ta Nodes meta3y tous, kai opote exoun k-1 koinous komvous dhmiourghse mia akmh
	/*	
		H Symmetrikh akmh 8a dhmiourgh8ei, otan oi komvoi sygkri8oun me antistrofh fora.
		Etsi telika 8a prokypsei ena mh-katey8hnomeno grafhma
	*/
	//Arxika dhmiourgoume ena thread pou elegxei ton ari8mo twn oloklhromenwn threads, wste na termatisei h diadikasia
	my_control_data->num_of_nodes = num_of_nodes;
	my_control_data->running_threads = &running_threads;
	my_control_data->total_threads = &total_threads;
	pthread_mutex_lock(&thread_finish_control);	//Arxika o mutex prepei na mplokarei th thread pou 8a dhmiourgh8ei
	
	rc = pthread_create(&control_thread, &attr, &control_threads, (void*)(my_control_data));
	if (rc){
		printf("cliquePercolationMethod: Error: return code from pthread_create() is %d\n", rc);
		return NULL;
		}
	
	//Arxikopoihse 3ana to attr
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	for(i=0; i<num_of_nodes; i++){
		//Fortwsh toy trexwn cell sta orismata
		my_data[i].clique_graph = clique_graph;
		my_data[i].current_cell = get_next_cell(my_cell_map, clique_graph);
		
		//Ekkinhsh thread
		//printf("cliquePercolationMethod: total_threads %d\n", total_threads);
		rc = pthread_create(&thread_array[i], &attr, &connect_cliques, (void*)(&my_data[i]));
		if (rc){
			printf("cliquePercolationMethod: Error: return code from pthread_create() is %d\n", rc);
			return NULL;
			}
		pthread_mutex_lock(&modify_num_of_threads);
		running_threads++;
		total_threads++;
		pthread_mutex_unlock(&modify_num_of_threads);
		
		//Elegxos plhthous trexontwn threads
		while(running_threads == MAX_NUM_OF_THREADS){
			pthread_cond_wait(&max_num_of_threads_reached, &allow_new_threads);
			}
		}
	
	printf("cliquePercolationMethod: Waiting for Threads to finish.\n");
	
	//3. Perimene ola threads na oloklhrwthoun
	pthread_join(control_thread, NULL);
	
	printf("cliquePercolationMethod: Successfully added Edges to Graph.\n");
	
	
	
	//C. YPOLOGISMOS SYNEKTIKWN YPOGRAFHMATWN KAI DHMIOURGIA COMMUNITIES
	my_communities = findCommunities(clique_graph);
	
	printf("cliquePercolationMethod: Communities found!\n");
	
	//D. APODESMEYSH MNHMHS KAI EPISTROFH
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&access_to_clique_graph);
	pthread_mutex_destroy(&access_to_id_count);
	pthread_mutex_destroy(&modify_num_of_threads);
	pthread_mutex_destroy(&allow_new_threads);
	pthread_mutex_destroy(&thread_finish);
	pthread_mutex_destroy(&thread_finish_control);
	pthread_cond_destroy(&max_num_of_threads_reached);
	destroy_cell_map(my_cell_map);
	free(my_control_data);
	free(my_data);
	free(thread_array);
	pam_destroyGraph(clique_graph);
	
	printf("cliquePercolationMethod: Finishing\n");
	
	return my_communities;
	}
	