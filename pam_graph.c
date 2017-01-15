//pam_graph.c
//Anapty3h logismikou - Ergasia 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pam_graph.h"
#include "pam_utilities.h"
#include "defines.h"


//Utilities
int hash(int id, int initial_size, int current_level, int split_bckt){
	int max_bucket = pow(2, current_level) * initial_size;
	int res = id%max_bucket;
	if(res < split_bckt){
		max_bucket = pow(2, (current_level+1)) * initial_size;
		return id%max_bucket;
		}
	else
		return res;
	}

int empty_graph(pam_graph *g) {
	//when i returned is larger or even to current size of hashT graph is empty
	//else use i to find first non-empty bucket;
	int i;
	for (i=0; i < g->current_size; i++)
		if (g->hashtable[i]->occupied_cells > 0)
			break;
	return i;
	}

void pam_print_graph(pam_graph* my_graph){
	
	int i, j, k;
	pam_bucket** hashtable;
	pam_bucket* current_bucket;
	pam_cell* current_cell;
	pam_lite_edge* current_edge;
	pam_relationship* current_relationship;
	
	
	//Elegxos eisodou
	if(my_graph == NULL){
		printf("pam_print_graph: Invalid input.\n");
		return;
		}
	
	hashtable = my_graph->hashtable;
	
	printf("PRINTING GRAPH\n");
	//Ektypwse Bucket
	for(i=0; i<my_graph->current_size; i++){
		printf("Bucket %d\n", i);
		current_bucket = hashtable[i];
		for(j=0; j<current_bucket->occupied_cells; j++){
			//printf("\tCell %d\n", j);
			current_cell = &(current_bucket->my_cells[j]);
			printf("\tNode id: %d\t", current_cell->my_node->id);
			
			
			for(k=0; k<current_cell->my_node->my_properties->num_of_properties; k++){
				printf("%d\t", *(int*)(current_cell->my_node->my_properties->my_properties[k]));
				}
				
				
			printf("\n");
			for(k=0; k<NUM_OF_TYPES; k++){
				if(current_cell->my_list[k].my_edges != NULL){
					printf("\t\tType %d\t", current_cell->my_list[k].my_type);
					current_edge = current_cell->my_list[k].my_edges;
					while(current_edge != NULL){
						printf("%d", current_edge->ending_id);
						current_relationship = current_edge->their_relationship;
						while(current_relationship != NULL){
							printf("(%d)", current_relationship->my_relationship_type);
							printf("(%f)", *(double*)((current_relationship->my_properties->my_properties[0])));
							current_relationship = current_relationship->next_relationship;
							}
						printf("\t");
						current_edge = current_edge->next_pam_edge;
						}
					printf("\n");
					}
				}
			
			}
		}
	printf("PRINTING GRAPH COMPLETE\n\n");
	}

cell_map* create_cell_map(){
	cell_map* my_cell_map;
	if( (my_cell_map = malloc(sizeof(cell_map))) == NULL){
		printf("create_cell_map: Unable to allocate memory\n");
		return NULL;
		}
	my_cell_map->bucket_num = 0;
	my_cell_map->cell_num = 0;
	return my_cell_map;
	}

void destroy_cell_map(cell_map* my_cell_map){
	if( my_cell_map == NULL){
		printf("destroy_cell_map: Invalid input\n");
		return;
		}
	else{
		free(my_cell_map);
		}
	}
	
pam_cell* get_next_cell(cell_map* my_cell_map, pam_graph* my_graph){
	pam_cell* my_cell;
	//Elegxos an exoume ftasei sto telos
	if(my_cell_map->bucket_num == -1){
		return NULL;
		}
	if(my_graph->num_of_nodes==0){
		return NULL;
		}
	
	my_cell = &(my_graph->hashtable[my_cell_map->bucket_num]->my_cells[my_cell_map->cell_num]);
	my_cell_map->cell_num++;
	if(my_cell_map->cell_num >= my_graph->hashtable[my_cell_map->bucket_num]->occupied_cells){
		my_cell_map->cell_num = 0;
		my_cell_map->bucket_num++;
		if(my_cell_map->bucket_num == my_graph->current_size){
			my_cell_map->bucket_num = -1;
			my_cell_map->cell_num = -1;
			}
		}
	return my_cell;
	}

void reset_cell_map(cell_map* my_cell_map){
	if( my_cell_map == NULL){
		printf("destroy_cell_map: Invalid input\n");
		return;
		}
	else{
		my_cell_map->bucket_num = 0;
		my_cell_map->cell_num = 0;
		}
	}
	
	
//Utilities: Hashtable Implementation
pam_lite_edge *pam_free_edge(pam_lite_edge *current_edge) {
	//destroyes current edge and returns next edge
	pam_relationship* current_relationship, *next_relationship;
	pam_lite_edge *next_edge;
	int i;

		current_relationship = current_edge->their_relationship;
		while(current_relationship != NULL){
			next_relationship = current_relationship->next_relationship;
			//Diagrafh Properties
			if (current_relationship->my_properties != NULL)
			{	for(i=0; i<current_relationship->my_properties->num_of_properties; i++){
					free(current_relationship->my_properties->my_properties[i]);
					}//Diagrafh stoixeiwn pinaka me properties
				if(current_relationship->my_properties->my_properties != NULL){
					free(current_relationship->my_properties->my_properties);//Diagrafh tou pinaka me properties
					}
			}
			free(current_relationship->my_properties);//Diagrafh domhs properties
			free(current_relationship);
			current_relationship = next_relationship;
		}
		next_edge = current_edge->next_pam_edge;
		free(current_edge);
		return next_edge;
		}

bool destroy_entity_list(entity_list* my_entity_list){
	pam_lite_edge* current_edge;
	
	if(my_entity_list == NULL){
		printf("destroy_entity_list: Invalid input\n");
		return false;
		}
	
	current_edge = my_entity_list->my_edges;
	
	while(current_edge != NULL){
		current_edge = pam_free_edge(current_edge);
		}
	my_entity_list->my_edges = NULL;
	}

pam_bucket *create_bucket(int num_of_cells){
	int i, j;
	node_type my_type;
	pam_bucket* pam_bucket_ptr;
	entity_list* entity_list_ptr;
	int entity_list_size = NUM_OF_TYPES;

	//Desmeysh mnhmhs gia Bucket
	if ( (pam_bucket_ptr = malloc(sizeof(pam_bucket))) == NULL)
	{	printf("create_bucket: Cannot allocate memory for bucket\n");
		return NULL;
		}
	
	//Arxikopoihsh metavlhtwn
	pam_bucket_ptr->current_size = num_of_cells;
	pam_bucket_ptr->occupied_cells = 0;
	
	//Dhmiourgia kai Arxikopoihsh twn cell
	if ( (pam_bucket_ptr->my_cells = malloc(sizeof(pam_cell) * num_of_cells)) == NULL)
	{	printf("Cannot allocate memory for bucket's cells\n");
		return NULL;
		}
	
	for(i=0; i<num_of_cells; i++){
		//Gia Entity list
		if ( (entity_list_ptr = malloc(entity_list_size * sizeof(entity_list))) == NULL)
		{	printf("create_bucket: Cannot allocate memory for bucket\n");
			for(j=0; j<i; j++){
				free(pam_bucket_ptr->my_cells[j].my_list);
				}
			free(pam_bucket_ptr->my_cells);
			free(pam_bucket_ptr);
			return NULL;
			}
		else{
			my_type = 0;
			for(j=0; j<entity_list_size; j++){
				entity_list_ptr[j].my_type = my_type++;
				entity_list_ptr[j].num_of_edges = 0;
				entity_list_ptr[j].my_edges = NULL;
				}
			}
		pam_bucket_ptr->my_cells[i].my_node = NULL;
		pam_bucket_ptr->my_cells[i].my_list = entity_list_ptr;
		}
	
	return pam_bucket_ptr;
	}

bool destroy_bucket(pam_bucket * my_bucket, char mode){
	/*
	PERIGRAFH:	Diagrafei to dosmeno bucket (mazi me ola ta periexomena tou an to mode einai 1)
	SE EPITYXIA:	Epistrefei true
	SE APOTYXIA:	Epistrefei false
	*/
	
	int i, j;
	pam_node* current_node;
	pam_relationship* current_relationship, * next_relationship;
	int num_of_occupied_cells = my_bucket->occupied_cells;
	int entity_list_size = NUM_OF_TYPES;
	
	//1. Diagrafh periexomenwn twn keliwn
	for(i=0; i<my_bucket->current_size; i++){
		if (mode == 1)
			if (i < num_of_occupied_cells)
			{	current_node = my_bucket->my_cells[i].my_node;
				//Diagrafh Properties
				if (current_node->my_properties != NULL)
				{	for(j=0; j<current_node->my_properties->num_of_properties; j++){
						free(current_node->my_properties->my_properties[j]);
						}//Diagrafh stoixeiwn pinaka me properties
					free(current_node->my_properties->my_properties);//Diagrafh tou pinaka me properties
				}
				free(current_node->my_properties);//Diagrafh domhs properties
				free(current_node);
				
			}
		
		//Diagrafh twn entity list
		for(j=0; j<entity_list_size; j++){
			destroy_entity_list(&(my_bucket->my_cells[i].my_list[j]));
			}
		free(my_bucket->my_cells[i].my_list);
		}
	
	//2. Diagrafh twn idiwn twn keliwn
	free(my_bucket->my_cells);
	
	//3. Diagrafh tou idiou tou bucket
	free(my_bucket);
	
	return true;
	}

bool adjust_bucket(pam_graph *graph, pam_bucket *bckt_ptr){
	int i, j, times_ovrflw;
	node_type my_type;
	entity_list* entity_list_ptr;
	int entity_list_size = NUM_OF_TYPES;
	
	if (bckt_ptr->occupied_cells <= graph->num_of_cells)
                times_ovrflw = 1;
        else
                times_ovrflw = bckt_ptr->occupied_cells / graph->num_of_cells + 1;
        bckt_ptr->current_size = times_ovrflw*graph->num_of_cells;
        if ((bckt_ptr->my_cells = realloc(bckt_ptr->my_cells, bckt_ptr->current_size*sizeof(pam_cell))) == NULL)
        {       printf("adjust_bucket: Cannot reallocate memory while readjusting empty overflowed buckets\n");
                // destroy_graph(graph);
                return false;
			}
		
        for (i=bckt_ptr->occupied_cells; i < bckt_ptr->current_size; i++){
			//Allocate Entity list
			if ( (entity_list_ptr = malloc(entity_list_size * sizeof(entity_list))) == NULL){
				printf("adjust_bucket: Cannot allocate memory for bucket\n");
				return false;
				}
			
			my_type = 0;
			for(j=0; j<entity_list_size; j++){
				entity_list_ptr[j].my_type = my_type++;
				entity_list_ptr[j].num_of_edges = 0;
				entity_list_ptr[j].my_edges = NULL;
				}
				
			bckt_ptr->my_cells[i].my_node = NULL;
			bckt_ptr->my_cells[i].my_list = entity_list_ptr;
			}
	return true;
	}

bool expand_hashtable(pam_graph *graph, int hashed_id){
	// Dexetai enan grafo kai th 8esh ston hashtable,
	// desmeuei xwro gia to bucket uperxeilishs kai
	// auksanei ton hashtable kata ena keli
	// epistrefei true or false
	int i, j, all_cells;
	int initial_size;
	int current_level;
	int split_bckt;
	int entity_list_size = NUM_OF_TYPES;
	node_type my_type;
	pam_bucket *splt_bckt_ptr, *new_bckt_ptr, *bucket_ptr;
	entity_list *entity_list_ptr;
	
	
	// Elegxos dedomenwn
	if (graph == NULL)
	{	printf("expand_hashtable: No graph found\n");
		return false;
	}
	
	// DESMEUSH MNHMHS
	// diplasiasmos hashtable otan eiserxomaste se neo epipedo
	if (graph->current_size == pow(2, graph->current_level)*graph->initial_size)
	{	if ((graph->hashtable = realloc(graph->hashtable, 2*graph->current_size*sizeof(pam_bucket*))) == NULL)
		{	printf("expand_hashtable: Cannot reallocate memory while doubling the hashtable\n");
			// destroy_graph(graph); //??????
			return false;
		}
		for (i=graph->current_size; i < 2*graph->current_size; i++)
		{	graph->hashtable[i] = NULL;
		}
		//graph->current_level++;
	}

	// desmeush mnhmhs mono gia ena bucket sto telos tou hashtable
	if((graph->hashtable[graph->current_size++] = create_bucket(graph->num_of_cells)) == NULL)
	{	printf("expand_hashtable: Error in create_bucket: ");
		return false;
	}
	for (i=0; i<graph->num_of_cells; i++)
		free(graph->hashtable[graph->current_size-1]->my_cells[i].my_list);
	graph->split_bckt++;

	// desmeush bucket uperxeilishs
	bucket_ptr = graph->hashtable[hashed_id];
	if ((bucket_ptr->my_cells = realloc(bucket_ptr->my_cells, (bucket_ptr->current_size+graph->num_of_cells)*sizeof(pam_cell))) == NULL)
	{	printf("expand_hashtable: Cannot reallocate memory during bucket overflow\n");
		// destroy_graph(graph); //??????
		return false;
	}
	for (i=bucket_ptr->current_size; i < bucket_ptr->current_size+graph->num_of_cells; i++)
	{	//Allocate Entity list
		if ( (entity_list_ptr = malloc(NUM_OF_TYPES * sizeof(entity_list))) == NULL){
			printf("adjust_bucket: Cannot allocate memory for bucket\n");
			return false;
			}
		
		my_type = 0;
		for(j=0; j<entity_list_size; j++){
			entity_list_ptr[j].my_type = my_type++;
			entity_list_ptr[j].num_of_edges = 0;
			entity_list_ptr[j].my_edges = NULL;
			}
		
		bucket_ptr->my_cells[i].my_node = NULL;
		bucket_ptr->my_cells[i].my_list = entity_list_ptr;
	}
	bucket_ptr->current_size += graph->num_of_cells;

	// DIXOTOMHSH BUCKET
    // diaforetiko apo bucket uperxeilishs
	splt_bckt_ptr = graph->hashtable[graph->split_bckt-1]; // auto tha moirasei tis eggrafes tou
	new_bckt_ptr = graph->hashtable[graph->current_size-1];
	if((new_bckt_ptr->my_cells = realloc(new_bckt_ptr->my_cells, splt_bckt_ptr->current_size*sizeof(pam_cell))) == NULL)
	{	printf("expand_hashtable: Cannot reallocate memory while adjusting new bucket's cells\n");
		// destroy_graph(graph); //??????
		return false;
	}
	// TELOS DESMEUSHS MNHMHS

//	new_bckt_ptr = graph->hashtable[graph->current_size-1];
//	splt_bucket_ptr = graph->hashtable[graph->split_bckt]; // auto tha moirasei tis eggrafes tou
	all_cells = splt_bckt_ptr->occupied_cells;
	new_bckt_ptr->occupied_cells = 0;
	splt_bckt_ptr->occupied_cells = 0;
	
	//Apo8hkeysh stoixeiwn gia th hash
	initial_size = graph->initial_size;
	current_level = graph->current_level;
	split_bckt =  graph->split_bckt;
	
	for (i=0; i < all_cells; i++)
	{	if ((hashed_id = hash(splt_bckt_ptr->my_cells[i].my_node->id, initial_size, current_level, split_bckt)) == graph->split_bckt-1)
		{	splt_bckt_ptr->my_cells[splt_bckt_ptr->occupied_cells].my_node = splt_bckt_ptr->my_cells[i].my_node;
			splt_bckt_ptr->my_cells[splt_bckt_ptr->occupied_cells].my_list = splt_bckt_ptr->my_cells[i].my_list;
			splt_bckt_ptr->occupied_cells++;
		}
		else
		{	new_bckt_ptr->my_cells[new_bckt_ptr->occupied_cells].my_node = splt_bckt_ptr->my_cells[i].my_node;
			new_bckt_ptr->my_cells[new_bckt_ptr->occupied_cells].my_list = splt_bckt_ptr->my_cells[i].my_list;
			new_bckt_ptr->occupied_cells++;
		}
		splt_bckt_ptr->my_cells[i].my_node == NULL;
		splt_bckt_ptr->my_cells[i].my_list == NULL;
	}
	// TELOS DIXOTOMISHS
	
	// split kai new bucket exoun to kathe ena megethos iso me all_cells.
	// perikoph adeiwn bucket uperxeilishs opou xreiazetai
	for (i=all_cells; i < splt_bckt_ptr->current_size ; i++)
	{	free(splt_bckt_ptr->my_cells[i].my_list);
		splt_bckt_ptr->my_cells[i].my_list = NULL;
	}
	
	if (adjust_bucket(graph, splt_bckt_ptr) == false)
		return false;
	if (adjust_bucket(graph, new_bckt_ptr) == false)
		return false;

	// aukshsh epipedou split
	if (graph->current_size == 2*pow(2, graph->current_level)*graph->initial_size)
	{	graph->current_level++;
		graph->split_bckt  = 0;	//3ekina to split apo thn arxh
	}
	/*
	int j;
	printf("current_size = %d\tcurrent_level = %d\tsplit_bckt = %d\n",  graph->current_size, graph->current_level, graph->split_bckt);
		
	for(i=0; i < graph->current_size; i++)
	{	printf("bucket: %d\tbucket size: %d/%d\n", i, graph->hashtable[i]->occupied_cells, graph->hashtable[i]->current_size);
		for(j=0; j < graph->hashtable[i]->occupied_cells; j++)
		{		printf(" cell: %d, id: %d ", j, graph->hashtable[i]->my_cells[j].my_node->id);
		}
		printf("\n");
	}
	*/
	return true;
	}


//Utilities: Fringe Implementation
fringe* create_fringe(char mode){
	//Dhmiourgei kai epistrefei mia domh typou fringe gia euresh mikroterou mhkous monopatiou
	//NA KALEITAI PANTA H destroy_fringe
	//mode: 0 for normal, 1 for weighted graphs
	fringe* my_fringe;

	if((my_fringe = malloc(sizeof(fringe))) == NULL ){
		printf("create_fringe: Unable to allocate memory\n");
		return NULL;
		}
	if((my_fringe->the_fringe = malloc(INITIAL_FRINGE_SIZE*sizeof(pam_cell**))) == NULL ){
		printf("create_fringe: Unable to allocate memory\n");
		return NULL;
		}
	if((my_fringe->temp_fringe = malloc(INITIAL_FRINGE_SIZE*sizeof(pam_cell**))) == NULL ){
		printf("create_fringe: Unable to allocate memory\n");
		return NULL;
		}
	if (mode > 0)	
	{	if((my_fringe->the_weight = malloc(INITIAL_FRINGE_SIZE*sizeof(double))) == NULL ){
			printf("create_fringe: Unable to allocate memory\n");
			return NULL;
		}
		if((my_fringe->temp_weight = malloc(INITIAL_FRINGE_SIZE*sizeof(double))) == NULL ){
			printf("create_fringe: Unable to allocate memory\n");
			return NULL;
		}
	}
	else
		my_fringe->the_weight = my_fringe->temp_weight = NULL;
	
	my_fringe->fringe_size = INITIAL_FRINGE_SIZE;
	my_fringe->the_fringe_counter = 0;
	my_fringe->temp_fringe_counter = 0;
	
	if((my_fringe->visited = malloc(INITIAL_FRINGE_SIZE*sizeof(int))) == NULL ){
		printf("create_fringe: Unable to allocate memory\n");
		return NULL;
		}
	my_fringe->visited_size = INITIAL_FRINGE_SIZE;
	my_fringe->visited_counter = 0;

	return my_fringe;
	}

void destroy_fringe(fringe* my_fringe){
	//Eley8erwnei th mnhmh pou exei katalabei mia domh fringe
	free(my_fringe->temp_weight);
	free(my_fringe->the_weight);
	free(my_fringe->the_fringe);
	free(my_fringe->temp_fringe);
	free(my_fringe->visited);
	free(my_fringe);
	}

bool find_intersect(fringe* starting_fringe, fringe* ending_fringe, char mode){
    int starting_counter=0, ending_counter=0;
    int starting_id=0, ending_id=0;
	bool found = false;
	double weight=0, start_w, end_w;
	int start_paths, end_paths, whatever;
	
	while( starting_counter < starting_fringe->the_fringe_counter){
		ending_counter = 0;
		starting_id = (starting_fringe->the_fringe[starting_counter])->my_node->id;
		while(ending_counter < ending_fringe->the_fringe_counter) {
			ending_id = (ending_fringe->the_fringe[ending_counter])->my_node->id;
			
			if(starting_id == ending_id){
				if (mode == 0)
					return true;
				found = true;
				start_w = starting_fringe->the_weight[starting_counter];
				end_w = ending_fringe->the_weight[ending_counter];
				if (mode == 1)
				{	if (start_w * end_w >= weight)
						weight = start_w * end_w;
				}
				else
				{	split_double(start_w, &whatever, &start_paths);
					split_double(end_w, &whatever, &end_paths);
					//printf("start_paths: %d, end_paths: %d\n", start_paths, end_paths);
					weight += end_paths * start_paths;
				}
			}
			ending_counter++;
			}
		starting_counter++;
		}
	if (mode > 0 && found == true)
		starting_fringe->the_weight[0] = weight;
	return found;
	}

bool visited_node(int new_node_id, int* visited_array, int visited_array_size){
	//Epistrefei an enas komvos yparxei mesa se dosmeno pinaka
	
	int i;
	
	//An o komvos exei episkeytei, epestrepse true
	for(i=0; i<visited_array_size; i++){
		if(new_node_id == visited_array[i]){
			return true;
			}
		}
		
	//Alliws einai h prwth episkepsh, opote epestrepse false
	return false;
	}
	
void set_constraints(pam_resultSet *resSet, int *constraints, char mode){
	resSet->constraints = constraints;
	
	if ((resSet->mode = mode) > 0)
	{	if (resSet->my_fringe->the_weight == NULL)
		{	if((resSet->my_fringe->the_weight = malloc(INITIAL_FRINGE_SIZE*sizeof(double))) == NULL ){
				printf("create_fringe: Unable to allocate memory\n");
				return;
			}
			if((resSet->my_fringe->temp_weight = malloc(INITIAL_FRINGE_SIZE*sizeof(double))) == NULL ){
				printf("create_fringe: Unable to allocate memory\n");
				return;
			}
		}
		resSet->my_fringe->the_weight[0] = 1.0 + 1.0/DECIMAL_DIGITS;
	
	}
	return;
	}

bool expand_fringe(pam_graph* my_graph, fringe* my_fringe, int* constraints, char mode){
	//Epekteinei to trexon synoro kata ena vhma
	//Epistrefei false se apotyxia
	//mode: 0 for normal, 1 for weighted graphs
	int temp_occupied = 0;	//Komvoi pou proste8hkan sto trexon epipedo ( == my_fringe->temp_fringe_counter)
	int i, j, new_size, current_neighbour_id, no_constraint;
	pam_cell* my_node_cell, *neighbour_node_cell;
	pam_lite_edge* current_edge;
	pam_cell** temper_fringe;
	double *temper_weight;
	pam_relationship *current_relation;
	
	//1. Epekteine ka8e ene3reynhto komvo sto synoro kai pros8ese ton sth lista me ta e3ereynh8enta
	for(i=0; i<my_fringe->the_fringe_counter; i++){
		my_node_cell = my_fringe->the_fringe[i];
		current_edge = my_node_cell->my_list[my_graph->my_node_type].my_edges;	//exapnd twn kobvwn idias ontothtas (p.x person-person, org-org)
		
		if (mode == GN_EDGES)
		{	if( visited_node(my_node_cell->my_node->id, my_fringe->visited, my_fringe->visited_counter+temp_occupied) )
				continue;
		}
		
		//2. Oso o komvos exei geitones, epixeirhse na tous pros8eseis sth lista
		while(current_edge != NULL){
			
			//2.5. Elegxos gia periorismous
			//eisagwgh kombwn pou uparxoun mesa sto constraints kai mono
			no_constraint = 1;
			if (constraints != NULL)
			{	no_constraint = 0;	
				for (j=0; constraints[j] != -1; j++)
					if (constraints[j] == current_edge->ending_id)
					{	no_constraint = 1;
						break;
					}
			}
		
			if (no_constraint)
			{	//3. Elegxos an o komvos exei hdh e3er3ynh8ei
				current_neighbour_id = current_edge->ending_id;
				if( !visited_node(current_neighbour_id, my_fringe->visited, my_fringe->visited_counter) ){
					
					//4. An einai anagkh, ay3hse to mege8os tou fringe (x INITIAL_FRINGE_SIZE )
					if(my_fringe->temp_fringe_counter == my_fringe->fringe_size){
						//Epektash fringe
						new_size = my_fringe->fringe_size * INITIAL_FRINGE_SIZE;
						if( (my_fringe->the_fringe = realloc(my_fringe->the_fringe, new_size * sizeof(pam_cell**))) == NULL){
							printf("expand_fringe: Unable to reallocate memory for my fringe\n");
							return false;
							}
						//Epektash temp fringe
						if( (my_fringe->temp_fringe = realloc(my_fringe->temp_fringe, new_size * sizeof(pam_cell**))) == NULL){
							printf("expand_fringe: Unable to reallocate memory for temp fringe\n");
							return false;
							}
						if (mode > 0)
						{	if( (my_fringe->the_weight = realloc(my_fringe->the_weight, new_size * sizeof(double))) == NULL){
								printf("expand_fringe: Unable to reallocate memory for my fringe\n");
								return false;
								}
							if( (my_fringe->temp_weight = realloc(my_fringe->temp_weight, new_size * sizeof(double))) == NULL){
								printf("expand_fringe: Unable to reallocate memory for temp fringe\n");
								return false;
								}
						}
						my_fringe->fringe_size = new_size;	//Enhmerwsh gia thn ay3hsh
						}
					
					//5.  An einai anagkh, ay3hse to mege8os tou visited (x INITIAL_FRINGE_SIZE )
					if(my_fringe->visited_size == my_fringe->visited_counter + temp_occupied){
						//Epektash visited
						new_size = my_fringe->visited_size * INITIAL_FRINGE_SIZE;
						if( (my_fringe->visited = realloc(my_fringe->visited, new_size * sizeof(int))) == NULL){
							printf("expand_fringe: Unable to reallocate memory for visited\n");
							return false;
							}
						my_fringe->visited_size = new_size;	//Enhmerwsh gia thn ay3hsh
						}

					if ( !visited_node(current_neighbour_id, my_fringe->visited + my_fringe->visited_counter, temp_occupied))
					{
				
						//6. Topo8ethsh tou kainouriou node sto synoro
						neighbour_node_cell = pam_lookup_cell(my_graph, current_neighbour_id);
						my_fringe->temp_fringe[my_fringe->temp_fringe_counter] = neighbour_node_cell;
						//6.5. Topo8ethsh tou kainouriou varous
						if (mode == TL_TRUST)
							my_fringe->temp_weight[my_fringe->temp_fringe_counter] = my_fringe->the_weight[i] * (*(double*)current_edge->their_relationship->my_properties->my_properties[0]);
						else if (mode == GN_BETWEEN)
							my_fringe->temp_weight[my_fringe->temp_fringe_counter] = my_fringe->the_weight[i];
						else if (mode == GN_EDGES)
						{	my_fringe->temp_weight[my_fringe->temp_fringe_counter] = my_node_cell->my_node->id;
							//printf("current %d neighbour %d\n", my_node_cell->my_node->id, neighbour_node_cell->my_node->id);
						}
						
						//7. Topo8ethsh kainouriou Node sth lista me tous gnwstous komvous
						if (mode != GN_EDGES)
						{	my_fringe->visited[my_fringe->visited_counter+temp_occupied] = current_neighbour_id;
							temp_occupied++;
						}
						my_fringe->temp_fringe_counter++;
						
					}
					else if (mode > 0 && mode < 3)
					{	for (j=0; j < my_fringe->temp_fringe_counter; j++)
							if (my_fringe->temp_fringe[j]->my_node->id == current_neighbour_id)
								break;
						if (mode == TL_TRUST)
						{	if (my_fringe->temp_weight[j] < my_fringe->the_weight[i] * (*(double*)current_edge->their_relationship->my_properties->my_properties[0]))
								my_fringe->temp_weight[j] = my_fringe->the_weight[i] * (*(double*)current_edge->their_relationship->my_properties->my_properties[0]);
						}
						else if (mode == GN_BETWEEN)
						{	my_fringe->temp_weight[j] += my_fringe->the_weight[i];
						}
					}
				}
			}
				//Epomenos komvos
				current_edge = current_edge->next_pam_edge;
				
		}
		if (mode == GN_EDGES)
		{	
			if(my_fringe->visited_size == my_fringe->visited_counter + temp_occupied){
						//Epektash visited
						new_size = my_fringe->visited_size * INITIAL_FRINGE_SIZE;
						if( (my_fringe->visited = realloc(my_fringe->visited, new_size * sizeof(int))) == NULL){
							printf("expand_fringe: Unable to reallocate memory for visited\n");
							return false;
							}
						my_fringe->visited_size = new_size;	//Enhmerwsh gia thn ay3hsh
						}
	
			my_fringe->visited[my_fringe->visited_counter+temp_occupied] = my_node_cell->my_node->id;
			temp_occupied++;
		}
	}
		
	//7. Pleon ta stoixeia tou the_fringe exoun e3ereynh8ei, opote 8etoume ws the_fringe to temp_fringe
	if(my_fringe->temp_fringe_counter==0){
		//printf("expand_fringe: Unable to find new Nodes\n");
		return false;
		}
	else{
		//enallagh listwn
		temper_fringe = my_fringe->the_fringe;
		my_fringe->the_fringe = my_fringe->temp_fringe;
		my_fringe->temp_fringe = temper_fringe;
		temper_weight = my_fringe->the_weight;
		my_fringe->the_weight = my_fringe->temp_weight;
		my_fringe->temp_weight = temper_weight;
		
		my_fringe->the_fringe_counter = my_fringe->temp_fringe_counter;
		my_fringe->visited_counter += temp_occupied;
		my_fringe->temp_fringe_counter = 0;
			//printf("temp_occupied: %d\n",temp_occupied);
		/*
			int i;
				printf("Fringe: ");
			for(i=0; i<my_fringe->the_fringe_counter; i++){
				printf("%d\t", my_fringe->the_fringe[i]->my_node->id);
				}
				printf("\n\n");
				*/
		return true;
		}
	}



//Utilities: Node List
pam_list* create_node_list(pam_graph *graph){
	//H synarthsh ayth dhmiourgei mia domh pou periexei olous tous komvous tou grafou
	//(xrhsimopoieitai apo to 2o epipedo)
	int i, j;
	pam_list* my_list;
	pam_bucket* current_bucket;
	node_entry* current_node_entry;
	
	//1. Dhmiourghse th lista
	if( (my_list = pam_create_list()) == NULL){
		return NULL;
		}
	
	//2. Pros8ese olous tous komvous tou grafou
	
	for(i=0; i<graph->current_size; i++){
		current_bucket = graph->hashtable[i];
		for(j=0; j<current_bucket->occupied_cells; j++){
		
			if ( (current_node_entry = malloc(sizeof(node_entry))) == NULL)
			{	printf("Cannot allocate memory for node entry\n");
				return NULL;
				}
			current_node_entry->my_element = &(current_bucket->my_cells[j]);
			current_node_entry->cb = 0;
			current_node_entry->delta = 0;
			current_node_entry->sigma = 0;
			current_node_entry->d = -1;
			current_node_entry->P = pam_create_list();
			pam_push_to_list(my_list, current_node_entry);
			}
		}
	
	return my_list;
	}
	
void node_list_reset(pam_list* my_node_list){
	//Kaleitai apo thn betweeness_centrality gia na mhdenisei tiw metavlhtes delta
	int i, size;
	node_entry* temp_node_entry;
	
	size = get_list_size(my_node_list);
	
	for(i=0; i<size; i++){
		temp_node_entry = pam_get_from_list(my_node_list, i);
		
		pam_destroy_list(temp_node_entry->P);
		temp_node_entry->P = pam_create_list();
		temp_node_entry->delta = 0;
		temp_node_entry->sigma = 0;
		temp_node_entry->d = -1;
		}
	}
	
void destroy_node_list(pam_list* my_node_list){
	int i;
	int size = get_list_size(my_node_list);
	node_entry* current_node_entry;
	
	for(i=0; i<size; i++){
		current_node_entry = (node_entry*)(pam_get_from_list(my_node_list, i));
		pam_destroy_list( current_node_entry->P );
		free(current_node_entry);
		}
	
	pam_destroy_list(my_node_list);
	}

node_entry* get_node_with_id(int id, pam_list* my_node_list){
	int i, size;
	node_entry*  my_node_entry;
	
	size = get_list_size(my_node_list);
	for(i=0; i<size; i++){
		my_node_entry = pam_get_from_list(my_node_list, i);
		if(my_node_entry->my_element->my_node->id == id){
			return my_node_entry;
			}
		}
	
	printf("get_node_with_id: Unable to find Node\n");
	return NULL;
	}
	
node_entry* get_node_with_offset(int offset, pam_list* my_node_list){
	int i;
	node_entry*  my_node_entry;
	
	if(offset >= get_list_size(my_node_list)){
		printf("get_node_with_offset: Invalid Input\n");
		return NULL;
		}
	my_node_entry = pam_get_from_list(my_node_list, offset);
	return my_node_entry;
	}



//Utilities: Dhmiourgia Properties
pam_properties* pam_createProperties(int number) {
	
	/**
	* Creates a properties object
	* number: the number of properties
	* return value: a properties object
	*/
	
	pam_properties* my_properties = malloc(sizeof(pam_properties));
	if( my_properties == NULL){
		printf("createProperties: Unable to create Properties.\n");
		return NULL;
		}
		
	my_properties->num_of_properties = number;
	if(number > 0){
		if( (my_properties->my_properties = malloc(number * sizeof(void*)) ) == NULL){
			printf("createProperties: Unable to create Properties.\n");
			return NULL;
			}
		}
	else{
		my_properties->my_properties = NULL;
		}
	return my_properties;
	
	}
	
pam_properties* setPkP_Properties() {
	pam_properties* prop;
	prop = pam_createProperties(0);
	
	return prop;
}

void pam_setStringProperty(char* property, int index, pam_properties* p) {

	/**
	 * Sets a string property "property" in position "index"
	 * property: the type of the property
	 * index: the position of the property
	 * p: the properties object
	 */
	
	int size_of_string = strlen(property) + 1;
	if (index >= 	p->num_of_properties || p->num_of_properties < 0){
		printf("setStringProperty: Invalid input.\n");
		return;
		}
	if( (p->my_properties[index] = malloc(size_of_string * sizeof(char)) ) == NULL){
		printf("setStringProperty: Unable to create Properties.\n");
		return;
		}
	strcpy(p->my_properties[index], property);
	}

void pam_setIntegerProperty(int property, int index, pam_properties* p) {
	
	/**
	* Sets an integer property "property" in position "index"
	* property: the value of the property
	* index: the position of the property
	* p: the properties object
	*/
	
	if (index >= 	p->num_of_properties || p->num_of_properties < 0){
		printf("setStringProperty: Invalid input.\n");
		return;
		}

	if( (p->my_properties[index] = malloc(sizeof(int)) ) == NULL){
		printf("setStringProperty: Unable to create Properties.\n");
		return;
		}

	*(int*)(p->my_properties[index]) = property;
		
	
	}

void pam_setDoubleProperty(double property, int index, pam_properties* p) {
	
	/**
	* Sets a double property "property" in position "index"
	* property: the value of the property
	* index: the position of the property
	* p: the properties object
	*/
	
	if (index >= 	p->num_of_properties || p->num_of_properties < 0){
		printf("setStringProperty: Invalid input.\n");
		return;
		}

	if( (p->my_properties[index] = malloc(sizeof(double)) ) == NULL){
		printf("setStringProperty: Unable to create Properties.\n");
		return;
		}

	*(double*)(p->my_properties[index]) = property;
		
	
	}

pam_node* pam_createNode(int id, pam_properties* p) {

	/**
	 * Creates a node with specific properties
	 * id: the id of the node
	 * p: the properties of the node
	 * return value: the requested node
	 */

	pam_node* my_node = malloc(sizeof(pam_node));
	if( my_node == NULL){
		printf("createNode: Unable to create Node.\n");
		return NULL;
		}
	my_node->id = id;
	my_node->my_properties = p;

	return my_node;
	}

pam_edge* pam_createEdge(int startID, int endID, pam_properties* p, node_type my_node_type, relationship_type my_relationship_type){
	/**
	 * Creates an edge with specific properties
	 * startID: the id of the start node
	 * endID: the id of the end node
	 * p: the properties of the edge
	 * return value: the requested edge
	 */
	pam_edge* my_edge = malloc(sizeof(pam_edge));
	if( my_edge == NULL){
		printf("createEdge: Unable to create Edge.\n");
		return NULL;
		}
	my_edge->my_edge = malloc(sizeof(pam_lite_edge));
	if( my_edge->my_edge == NULL){
		printf("createEdge: Unable to create Edge.\n");
		return NULL;
		}
	
	my_edge->my_edge->their_relationship = malloc(sizeof(pam_relationship));
	if(my_edge->my_edge->their_relationship == NULL){
		free(my_edge->my_edge);
		free(my_edge);
		printf("createEdge: Unable to create Edge.\n");
		return NULL;
		}
	
	my_edge->starting_id = startID;
	my_edge->my_edge->ending_id = endID;
	my_edge->my_node_type = my_node_type;
	my_edge->my_edge->their_relationship->my_relationship_type = my_relationship_type;
	my_edge->my_edge->their_relationship->my_properties = p;
	my_edge->my_edge->their_relationship->next_relationship = NULL;
	my_edge->my_edge->next_pam_edge = NULL;
	return my_edge;
	}



//Zhtoymenes synarthseis
//Zhtoymenes synarthseis: Epipedo 1 (pam_graph)
pam_graph* pam_createGraph(int initial_hashtable_size, int num_of_cells, node_type my_node_type){

	//printf("pam_createGraph\n");
	pam_graph *my_graph;	//Deikths sth desmevmenh mnhmh gia to grafo
	pam_bucket **my_hashtable;	//Deikths sth desmevmenh mnhmh gia to hash table
	int* my_num_of_edges_type;
	
	//1. Elegxos dedomenwn
	if(initial_hashtable_size<=0){
		printf("Invalid initial size. Graph could not be created.\n");
		return NULL;
	}
	else if(num_of_cells<=0){
		printf("Invalid number of cells per bucket. Graph could not be created.\n");
		return NULL;
	}
	
	//2. Desmeysh mnhmhs
	//Grafos
	if ( (my_graph = malloc(sizeof(pam_graph))) == NULL)
	{	printf("Cannot allocate memory for graph\n");
		return NULL;
	}
	
	//Hash table
	if ( (my_hashtable = malloc(sizeof(pam_bucket*) * initial_hashtable_size)) == NULL)
	{	printf("Cannot allocate memory for hash table\n");
		free(my_graph);
		return NULL;
	}
	
	//Buckets
	int i;
	for(i=0; i<initial_hashtable_size; i++){
		my_hashtable[i] = create_bucket(num_of_cells);	//H create_bucket() dhmiourgei kai arxikopoiei  kai ta antistioxa cells
		}

	//Ari8mos Edges
	if ( (my_num_of_edges_type = calloc(NUM_OF_TYPES, sizeof(int))) == NULL)
	{	printf("Cannot allocate memory for counter of edges\n");
		return NULL;
	}
	
	//3. Dhmioyrgia toy grafou
	//Grafos
	my_graph->my_node_type = my_node_type;
	my_graph->initial_size = initial_hashtable_size;
	my_graph->current_size = initial_hashtable_size;
	my_graph->num_of_cells = num_of_cells;
	my_graph->current_level = 0;
	my_graph->hashtable = my_hashtable;
	my_graph->split_bckt = 0;
	my_graph->num_of_nodes = 0;
	my_graph->num_of_edges = 0;
	my_graph->num_of_edges_type = my_num_of_edges_type;
	my_graph->betweeness_centrality_results = NULL;
	
	//5. Epistrofh deikth
	return my_graph;
	}

bool pam_destroyliteGraph(pam_graph* my_graph, char mode){
	// mode: 0 is soft mode(excludes nodes), 1 destroys the whole graph

	//printf("pam_destroyGraph\n");
	//Diagrafh twn bucket 
	int i;
	for(i=0; i< my_graph->current_size; i++){
		//printf("pam_destroyGraph bucket: %d/%d\n", i, my_graph->current_size);
		if(destroy_bucket(my_graph->hashtable[i], mode) == false){
			return false;
		}
	}
	
	//Diagrafh apotelesmatwn Centrality
	free(my_graph->betweeness_centrality_results);
	
	//Diagrafh hashtable
	free(my_graph->hashtable);
	
	//Diagrafh metrhtwn
	free(my_graph->num_of_edges_type);
	
	//Diagrafh graph
	free(my_graph);
	
	return true;
	}

bool pam_destroyGraph(pam_graph* my_graph) {
	return pam_destroyliteGraph(my_graph, 1);
	}

bool pam_insertNode(pam_graph* my_graph, pam_node* my_node){
	
	//1. Eyresh bucket
	//8ewroume oti o my_node exei apo8hkeytei sth mnhmh prin thn klhsh ths synarthshs kai 8a synexisei na einai ekei mexri na ton afhsoume
	int hashed_id = hash(my_node->id, my_graph->initial_size,  my_graph->current_level, my_graph->split_bckt); 
	int num_of_cells = my_graph->hashtable[hashed_id]->occupied_cells;
	
	/*
	printf("\nnum_of_cells: %d/%d\n", num_of_cells, my_graph->hashtable[hashed_id]->current_size);
	printf("hashed_id %d\n", hashed_id);
	printf("my_graph->current_level %d\n", my_graph->current_level);
	printf("my_node->id %d\n", my_node->id);
	*/
	
	//2. Elegxos an einai gemato
	if(num_of_cells == my_graph->hashtable[hashed_id]->current_size){
		
		//2 i. An nai, epekteinoume to hashtable kai dhmiourgoume to overflow bucket
		if (expand_hashtable(my_graph, hashed_id) == false){
			printf("pam_insertNode: Error expanding hashtable\n");
			return false;
		}
		return  pam_insertNode(my_graph,  my_node);	//Kaloume anadromika wste na diathrhsoume th symmetria ari8mou bucket kai overflow bucket
	}

	//3. Eyresh 8eshs topo8ethshs (me xrhsh bubblesort)
	pam_cell* cell_table = my_graph->hashtable[hashed_id]->my_cells;	
	int i;
	
	//3 i. Swaps mexri na vre8ei sth swsth 8esh
	for(i=0; i<num_of_cells; i++){
		if(cell_table[num_of_cells - i - 1].my_node->id < my_node->id){
			break;
		}
		cell_table[num_of_cells - i].my_node = cell_table[num_of_cells - i - 1].my_node;
	}
	cell_table[num_of_cells - i].my_node = my_node;
	
	//4. Au3hsh tou metrhth gia katelhlhmena kelia
	my_graph->hashtable[hashed_id]->occupied_cells++;
	my_graph->num_of_nodes++;
	return true;
	}

bool pam_insertEdge(pam_graph* my_graph, pam_edge* my_edge){
	int i, j;
	
	
	//1. Elegxos dedomenwn
	if(my_graph==NULL || my_edge->starting_id<0 || my_edge==NULL){
		printf("insertEdge: Invalid input\n");
	}
	/*
	if(my_graph->my_node_type != my_edge->my_node_type){
		printf("insertEdge: Invalid input. The Edge has benn created for a Graph of type %d\n", my_edge->my_node_type);
		printf("and this Graph is of type %d\n", my_graph->my_node_type);
		//??? Mporoume na diagrafoume thn Edge
		return false;
		}
	*/
	
	//2. Euresh cell pou periexei ton komvou
	pam_cell* target_cell = pam_lookup_cell(my_graph, my_edge->starting_id);
	if(target_cell==NULL){
		//printf("insertEdge: Unable to find Node with id %d so it will be deleted\n", my_edge->starting_id);
		//Destroy edge my_properties
		int num_of_prop = my_edge->my_edge->their_relationship->my_properties->num_of_properties;
		for(i=0; i<num_of_prop; i++){
			free(my_edge->my_edge->their_relationship->my_properties->my_properties[i]);
			}
		free(my_edge->my_edge->their_relationship->my_properties);
		free(my_edge->my_edge->their_relationship);
		free(my_edge->my_edge);
		free(my_edge);
		return false;
	}
	
	//3. Pros8hkh akmis
	//Euresh akmhs pou na katalhgei sto idio id me th dosmenh edge
	pam_lite_edge* last_edge = target_cell->my_list[my_edge->my_node_type].my_edges;
	
	//An yparxei hdh edge pros to dosmeno Node, pros8etoyme thn kainouria syndesh
	while(last_edge != NULL){
		if(last_edge->ending_id == my_edge->my_edge->ending_id){
			pam_relationship** last_relationship, *current_relationship;
			last_relationship = &(last_edge->their_relationship);
			current_relationship = last_edge->their_relationship;
			
			
			//An yparxei hdh edge, psaxnoume, an yparxei idiou typou
			while(current_relationship != NULL){
				//An yparxei hdh kataxwrhsh me ton dosmeno typo akmhs, ananewsh me kainouria stoixeia
				if(current_relationship->my_relationship_type == my_edge->my_edge->their_relationship->my_relationship_type){
					//Diagrafh prohgoumenwn Properties
					for(i=0; i<current_relationship->my_properties->num_of_properties; i++){
						free(current_relationship->my_properties->my_properties[i]);
						}
					free(current_relationship->my_properties->my_properties);
					free(current_relationship->my_properties);
					
					//Apo8hkeysh newn stoixeiwn
					current_relationship->my_properties = my_edge->my_edge->their_relationship->my_properties;
					
					//printf("insertEdge: Edge existed already. It has been updated.\n");
					
					free(my_edge->my_edge->their_relationship);
					free(my_edge->my_edge);
					free(my_edge);
					return true;
				}
				last_relationship = &(current_relationship->next_relationship);
				current_relationship = current_relationship->next_relationship;
			}
			//An den yparxei h dosmenh sxesh, pros8hkh stis yparxouses sxeseis
			my_edge->my_edge->their_relationship->next_relationship = last_edge->their_relationship;
			last_edge->their_relationship = my_edge->my_edge->their_relationship;
			//next_relationship =  my_edge->their_relationship;
			free(my_edge->my_edge);//FREE THE GIVEN EDGE!!! (h sxesh den diagrafetai)
			free(my_edge);
			return true;
		}
		last_edge = last_edge->next_pam_edge;
		//next_edge = next_edge->next_pam_edge;
	}
	//An den yparxei syndesh, tote apo8hkeyoume thn kainouria syndesh
	my_edge->my_edge->next_pam_edge = target_cell->my_list[my_edge->my_node_type].my_edges;
	target_cell->my_list[my_edge->my_node_type].my_edges = my_edge->my_edge;
	target_cell->my_list[my_edge->my_node_type].num_of_edges++;
	
	
	/*
	printf("Node %d\tEdges: ", starting_id);
	while(last_edge!=NULL){
		printf("%d\t", last_edge->ending_id);
		last_edge = last_edge->next_pam_edge;
		}
		printf("\n\n");
	*/
	my_graph->num_of_edges_type[my_edge->my_node_type] += 1;
	my_graph->num_of_edges++;
	
	//Diagrafh tou Wrapper
	free(my_edge);
	
	return true;
	}

pam_node* pam_lookupNode(pam_graph* my_graph, int my_id){
	pam_cell* my_cell = pam_lookup_cell(my_graph, my_id);
	if(my_cell==NULL){
		//printf("pam_lookupNode: Unable to find Node.\n");
		return NULL;
		}
	else{
		return my_cell->my_node;
		}
	}

pam_cell* pam_lookup_cell(pam_graph* my_graph, int my_id){
	int i, j;
	return pam_offsets(my_graph, my_id, &i, &j);
	}

pam_cell* pam_offsets(pam_graph* my_graph, int my_id, int *i, int *j){
	//1.Elegxos an einai adeios o grafos
	if(my_graph==NULL){
		printf("pam_offsets : Empty Graph\n");
		return NULL;
	}
	
	//1. Vres to bucket pou periexei ton komvo
	int temp_hash = hash(my_id, my_graph->initial_size,  my_graph->current_level, my_graph->split_bckt);
	*i = temp_hash;
	
	//8ewroume pws to apotelesma einai apodekto bucket, ane3arthta an to id yparxei h' oxi, afou h hash epistrefei me vash modulo
	
	//2. Mpainoume sto bucket kai kanoume dyadikh anazhthsh, gia na vroume ton komvo
	pam_bucket* my_bucket = my_graph->hashtable[temp_hash];
	
	int starting_cell = 0;
	int ending_cell = my_bucket->occupied_cells-1;
	int middle_cell =  (ending_cell + starting_cell)/2;
	pam_cell* temp_cell = NULL;
	
	if(my_bucket->occupied_cells == 0){
		//printf("lookupNode: Unable to find Node\n");
		return NULL;
		}
	
	//Dixotamhsh mexri na ftasoume se megethos 2 h' na vroume lysh 
	while(starting_cell != middle_cell && ending_cell != middle_cell){
		temp_cell = &(my_bucket->my_cells[middle_cell]);
		//An to keli einai to zhtoumeno, apistrefoume deikth sto Node
		if(temp_cell->my_node->id == my_id){
			*j = middle_cell;
			return temp_cell;
		}
		//An to id sto keli einai mikrotero apo to zhtoumeno, psaxnoume ta epomena
		else if(temp_cell->my_node->id < my_id){
			starting_cell = middle_cell;
		}
		//An to id sto keli einai megalytero apo to zhtoumeno, psaxnoume ta prohgoumena
		else{
			ending_cell = middle_cell;
		}
		middle_cell = (ending_cell + starting_cell)/2;      //(ending_cell - starting_cell+1)/2;
	}
	/*
	printf("my_id: %d\n", my_id);
	printf("starting_cell: %d\tid: %d\n", starting_cell, my_bucket->my_cells[starting_cell].my_node->id);
	printf("ending_cell: %d\tid: %d\n", ending_cell, my_bucket->my_cells[ending_cell].my_node->id);
	printf("middle_cell: %d\tid: %d\n", middle_cell, my_bucket->my_cells[middle_cell].my_node->id);
	*/
	
	//Elegxos an einai kapoio apo ta akra tou diasthmatos
	if(my_bucket->my_cells[starting_cell].my_node->id == my_id){
			temp_cell = &(my_bucket->my_cells[starting_cell]);
			*j = starting_cell;
			return temp_cell;
		}
	else if(my_bucket->my_cells[ending_cell].my_node->id == my_id){
			temp_cell = &(my_bucket->my_cells[ending_cell]);
			*j = ending_cell;
			return temp_cell;
		}
	else{
		//printf("pam_offsets: Unable to find Node\n");
		return NULL;
		}
	}

double pam_reachNode2(pam_graph* my_graph, int my_starting_id, int my_ending_id, char mode){
	pam_cell* starting_node_cell, *ending_node_cell;

	//Den kratame diadromh, para mono apostash
	//0. Elegxos dedomenwn
	if(my_graph==NULL || my_starting_id < 0 || my_ending_id < 0){
		printf("pam_reachNode1: Invalid input.\n");
		return -1;
		}
	if(my_starting_id == my_ending_id){
		return 0;
		}

	//1. Eyresh Node
	starting_node_cell = pam_lookup_cell(my_graph, my_starting_id);
	ending_node_cell = pam_lookup_cell(my_graph, my_ending_id);
	if(starting_node_cell==NULL || ending_node_cell == NULL){
		printf("pam_reachNode1: Unable to find Node.\n");
		return -1;
		}

	//2. 
	fringe* starting_fringe = create_fringe(mode);
	fringe* ending_fringe = create_fringe(mode);
	bool memory_error = false, found_path = false, no_new_nodes = false;
	double current_distance = 0;
	
	if(starting_fringe==NULL){
		printf("pam_reachNode1: An error has occured.\n");
		if(ending_fringe!=NULL){
			destroy_fringe(ending_fringe);
			}
		memory_error = true;
		}
	if(ending_fringe==NULL){
		printf("pam_reachNode1: An error has occured.\n");
		if(starting_fringe!=NULL){
			destroy_fringe(starting_fringe);
			}
		memory_error = true;
		}
	if(memory_error){
		return -1;
		}

	printf("pam_reachNode2: my_starting_id %d \t my_ending_id: %d\n", my_starting_id, my_ending_id);
	//Arxikopoihsh twn komvwn poy exoume episkeytei
	starting_fringe->the_fringe[0] = starting_node_cell;
	ending_fringe->the_fringe[0] = ending_node_cell;
	starting_fringe->the_fringe_counter = 1;
	ending_fringe->the_fringe_counter = 1;
	starting_fringe->visited[0] = my_starting_id;
	ending_fringe->visited[0] = my_ending_id;
	starting_fringe->visited_counter = 1;
	ending_fringe->visited_counter = 1;
	if (mode == 1){
		starting_fringe->the_weight[0] = 1;
		ending_fringe->the_weight[0] = 1;
		}
	else if (mode == 2){
		starting_fringe->the_weight[0] = 1.0 / DECIMAL_DIGITS;
		ending_fringe->the_weight[0] = 1.0 / DECIMAL_DIGITS;
		}

	while(!no_new_nodes && !found_path){
		//i. Epekteinoume apo arxh
		if(expand_fringe(my_graph, starting_fringe, NULL, mode) == false){
			no_new_nodes = true;
			current_distance = INFINITY_REACH_NODE;
			continue;
			}
		//ii. Elegxoume gia tomh
		if(find_intersect(starting_fringe, ending_fringe, mode) == true){
			found_path = true;
			current_distance =  (current_distance*2) + 1;
			continue;
			}
		//iii. Epekteinoume apo telos
		if(expand_fringe(my_graph, ending_fringe, NULL, mode) == false){
			no_new_nodes = true;
			current_distance = INFINITY_REACH_NODE;
			continue;
			}
		//iv. Elegxoume gia tomh
		if(find_intersect(starting_fringe, ending_fringe, mode) == true){
			found_path = true;
			current_distance =  (current_distance*2) + 2;
			continue;
			}
		
		current_distance++;
		}
		
	if	(mode > 0 && current_distance != INFINITY_REACH_NODE)
		current_distance = starting_fringe->the_weight[0];
	
	destroy_fringe(starting_fringe);
	destroy_fringe(ending_fringe);
	return current_distance;
	}

int pam_reachNode1(pam_graph* my_graph, int my_starting_id, int my_ending_id){
	return (int)pam_reachNode2(my_graph, my_starting_id, my_ending_id, 0);
	}

pam_resultSet* pam_reachNodesN(pam_graph* my_graph, int my_id){
	pam_resultSet *resSet;
	pam_cell* starting_node_cell;
	
	//1. DESMEYSH MNHMHS
	if((resSet = malloc(sizeof(pam_resultSet))) == NULL)
	{	printf("pam_reachNodes: cannot allocate resultSet memory\n");
		return NULL;
	}
	if((resSet->my_fringe = create_fringe(0)) == NULL)
	{	printf("pam_reachNodes: cannot allocate fringe memory\n");
		return NULL;
	}
	
	//2. euresh arxikou kombou
	if ((starting_node_cell = pam_lookup_cell(my_graph, my_id) )== NULL)
	{	printf("in pam_reachnodeN\n");
		return NULL;
	}

	//3 arxikopoihsh resultSet
	resSet->my_fringe->the_fringe[0] = starting_node_cell;
	resSet->my_fringe->the_fringe_counter = 1;
	resSet->my_fringe->visited[0] = my_id;
	resSet->my_fringe->visited_counter = 1;
	resSet->my_graph = my_graph;
	resSet->constraints = NULL;
	resSet->mode = 0;
	
	
	//4. Pros8hkh twn geitonwn
	/*if (expand_fringe(resSet->my_graph, resSet->my_fringe) == false)
		//An apotyxei, den yparxei syndesh
		{	printf("pam_next: No more relationships to show\n");
			destroy_fringe(resSet->my_fringe);
			free(resSet);
			return NULL;
			} */
	resSet->dist = 0;
	resSet->current_node = 0;
	
	return resSet;
	}

bool pam_next(pam_resultSet* my_resultSet, pam_result* res){
	pam_node *next_node;
	pam_cell* next_node_cell;
	int mode = my_resultSet->mode;
	
	//1. Elegxos dedomenwn
	if(res==NULL){
		printf("pam_next: Invalid result pointer.\n");
		return false;
		}
	if(my_resultSet==NULL){
		printf("pam_next: Invalid result set. Perhaps already finished...\n");
		return false;
		}
		
	/*
	int i;
	printf("My Fringe\n");
	for(i=0; i<my_resultSet->my_fringe->the_fringe_counter; i++ ){
		printf("%d\t", my_resultSet->my_fringe->the_fringe[i]->id);
		}
	printf("\n\n");
	*/
	do{
		//An den yparxoun komvoi pou exoun e3ereynh8ei kai den tous exoume episkeytei
		if(my_resultSet->current_node == my_resultSet->my_fringe->the_fringe_counter){
			if (expand_fringe(my_resultSet->my_graph, my_resultSet->my_fringe, my_resultSet->constraints, my_resultSet->mode) == false)
			//An apotyxei, den yparxei syndesh
			{	//printf("pam_next: No more relationships to show\n");
				return false;
				}
			my_resultSet->dist++;
			my_resultSet->current_node = 0;
			}
	
		next_node_cell = my_resultSet->my_fringe->the_fringe[my_resultSet->current_node];
		res->id = next_node_cell->my_node->id;
		if (mode != GN_EDGES)
			res->distance = my_resultSet->dist;
		else
		{	res->distance = (int)my_resultSet->my_fringe->the_weight[my_resultSet->current_node];
			mode = DEFAULT;
		}
		if (mode == GN_BETWEEN && (int)my_resultSet->my_fringe->the_weight[my_resultSet->current_node] != 0)
			mode = DEFAULT;
		my_resultSet->current_node++;
		} 
	while (mode != DEFAULT);
	
	return true;
	}

void pam_destroy_resultSet(pam_resultSet *my_resultSet){
	destroy_fringe(my_resultSet->my_fringe);
	//free(my_resultSet->constraints);
	free(my_resultSet);
	//printf("pam_destroy_resultSet: Result set destroyed\n");	
	}
