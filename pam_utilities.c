//pam_utilities.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "defines.h"
#include "pam_graph.h"
#include "data_storage_utilities.h"

//UTILITIES: DIAXEIRHSH CCs
void **create_tableOf_g(pam_graph *g, size_t size){
	int i, j;
	void **table;
	
	if ((table = malloc(g->current_size*sizeof(void*))) == NULL)
	{	printf("Cannot allocate memory for table\n");
		return NULL;
	}
	
	for (i=0; i< g->current_size; i++)
	{	
		if ((table[i] = calloc(g->hashtable[i]->occupied_cells,size)) == NULL)
		{	printf("Cannot allocate memory for table[i]\n");
			return NULL;
		}
		
	}
	
	return table;
	}

void *table_init(pam_graph *g, size_t size, void** table, int *CC, int endOfCC, void *value){
	int id, i, j;
	

	for (id=0; id < endOfCC; id++)
	{	if (pam_offsets(g, CC[id], &i, &j) == NULL)
		{	printf("by table_init\n");
			return NULL;
		}
		memcpy(table[i] + j, value, size);
	}
	return value;
	}

void destroy_table(pam_graph *g, void **table){
	int i;
	
	for (i=0; i< g->current_size; i++)
	{	
		free(table[i]);
		
	}
	free(table);
	
	}

int findCC(int *i, int *j, pam_resultSet *resSet, int *constraints){
	//vriskei ena grafhma gia kathe klish
	//epistrefei  0 otan den uparxei allo grafhma
	pam_result res;
	char start_bfs=0, no_constraint;
	int k, new_size;
	pam_cell *cell_ptr;
	resSet->dist = 0;
	
	
	while ( pam_next(resSet, &res) != false){}
	// vres enan komvo pou den exoume episkeftei psaxnontas seiriaka olous tous komvous
	for (; *i < resSet->my_graph->current_size; (*i)++)
	{	for (; *j < resSet->my_graph->hashtable[*i]->occupied_cells; (*j)++)
		{	cell_ptr = &(resSet->my_graph->hashtable[*i]->my_cells[*j]);
			no_constraint=0;
			if (constraints == NULL)
				no_constraint=1;
			else
				for (k=0; constraints[k] != -1; k++)
					if (constraints[k] == cell_ptr->my_node->id)
						no_constraint = 1;
			
			if (no_constraint)
				if (!visited_node(cell_ptr->my_node->id, resSet->my_fringe->visited, resSet->my_fringe->visited_counter))
				{	//printf("22: i: %d, j: %d, node_my_node->id: %d\n", *i, *j, cell_ptr->my_node->id);
					start_bfs = 1;
					break;
				}
		}
		if (start_bfs)
			break;
		*j = 0;
	}
	//episkeuthkame olous tous kombous tou graphou. Telos anazhthshs
	if (*i == resSet->my_graph->current_size)
		return 0;
	
	// epanazrxikopoihsh metablhtwn gia epomeno grafhma
	// kratame ta visited kai to distance
	
	resSet->my_fringe->the_fringe[0] = cell_ptr;
	resSet->my_fringe->the_fringe_counter = 1;
	resSet->my_fringe->temp_fringe_counter = 0;
	if (resSet->my_fringe->visited_counter == resSet->my_fringe->visited_size)
	{	new_size = resSet->my_fringe->visited_size * INITIAL_FRINGE_SIZE;
		if( (resSet->my_fringe->visited = realloc(resSet->my_fringe->visited, new_size * sizeof(int))) == NULL)
		{	printf("findCC: Unable to reallocate memory for visited\n");
			return false;
		}
		resSet->my_fringe->visited_size = new_size;
	}
	resSet->my_fringe->visited[resSet->my_fringe->visited_counter] = cell_ptr->my_node->id;
	
	//resSet->dist = 0;
	resSet->current_node = 0;
	
	//vrhkame grafhma alla uparxoun kai alloi komvoi pros eksereunish
	return 1;
	}

int findCCs(pam_graph *g, int *maxCCsize, int *numofCCs, int *constraints, unsigned char **belongsCC){
	pam_result res;
	pam_resultSet *resSet;
	int i, j, k;
	int id, CCsize;
	int prev_counter = 0, flag = 0;
	char last_CC = 0;
	unsigned char num=0;
	
	
	//Vres to prwto mh keno bucket
	i = 0;
	j = 0;
	if (constraints == NULL){
		while (i < g->current_size && g->hashtable[i]->occupied_cells == 0){
			i++;
			}
		}
	else{
		for (i=0; i < g->current_size; i++){
			for (j=0; j < g->hashtable[i]->occupied_cells; j++){
				for (k=0; constraints[k] != -1; k++){
					if (constraints[k] == g->hashtable[i]->my_cells[j].my_node->id){
						flag = 1;
						break;
						}
					}
				if (flag == 1){
					break;
					}
				}
			if (flag == 1){
				break;
				}
			}
		}	
		
	if (i >= g->current_size){
		printf("findsCCs: Empty Graph\n");
		return 1;
		}
	
		
	id = g->hashtable[i]->my_cells[j].my_node->id;
	//printf("NODE ID: %d\n", id);
	resSet = pam_reachNodesN(g, id);
	set_constraints(resSet, constraints, 0);
	
	while(1){
		last_CC = findCC(&i, &j, resSet, constraints);
		CCsize = resSet->my_fringe->visited_counter - prev_counter;
		if (CCsize > *maxCCsize)
			*maxCCsize = CCsize;
		(*numofCCs)++;
		num++;
		
		if (belongsCC != NULL)
			table_init(g, sizeof(unsigned char), (void**)belongsCC, resSet->my_fringe->visited+prev_counter, CCsize, (void*)&num);
		prev_counter = resSet->my_fringe->visited_counter;
		
		if (last_CC == 0)
		{	pam_destroy_resultSet(resSet);
			return 0;
		}
		resSet->my_fringe->visited_counter++;
		}
	}

int get_degree(pam_cell cell){
	int degree = 0, k;
	pam_lite_edge *edge_ptr;
	for (k=0; k < NUM_OF_TYPES; k++)
	{	edge_ptr = cell.my_list[k].my_edges;
		while(edge_ptr != NULL)
		{	edge_ptr = edge_ptr->next_pam_edge;
			degree++;
		}
	}	
	return degree;		
	}

double get_difference(char *birth1, char *birth2){
	//format "yy-mm-dd"
	double year1, month1, day1, month2, day2, year2, age1, age2, temp, differs=0;
	sscanf(birth1, "%lf-%lf-%lf", &year1, &month1, &day1);
	sscanf(birth2, "%lf-%lf-%lf", &year2, &month2, &day2);
	
	//year1 must be smaller year
	if (year1 > year2)
	{	temp = year1; year1 = year2; year2 = temp;
		temp = month1; month1 = month2; month2 = temp;
		temp = day1; day1 = day2; day2 = temp;
	}
	
	differs = year2 - year1;
	differs += (month2-1)/12 - (month1-1)/12;
	differs += (day2-1)/365.25 - (day1-1)/365.25;
	return differs;
	}

void split_double(double dbl, int *integer, int *decimal){
	//splits double into its integer and decimal part	
	*integer = (int)dbl;
	double mul = DECIMAL_DIGITS * (dbl-(int)dbl);
	//printf("-%.50lf\n", dbl);
	*decimal = round(mul);
	}



//UTILITIES: DIAXEIRHSH ARXEIWN CSV
typedef enum {int_type=0, string_type} property_type;

relationship_type determine_relationship_type(char* starting_line){
	
	
	//Elegxos eisodou
	if(starting_line == NULL){
			printf("determine_relationship_type: Invalid input.");
			return invalid_relationship;
		}
	
	//Euresh typou
	if(	!strcmp(starting_line, "Post.id|Person.id\n") || 
		!strcmp(starting_line, "post_id|person_id\r\n") || 
		!strcmp(starting_line, "Post.id|Person.id\r\n")){
		return post_hasCreator_person;
		}
	if(	!strcmp(starting_line, "Forum.id|Post.id\n") || 
		!strcmp(starting_line, "forum_id|post_id\r\n") || 
		!strcmp(starting_line, "Forum.id|Post.id\r\n")){
		return forum_containerOf_post;
		}
	if(	!strcmp(starting_line, "Forum.id|Person.id|joinDate\n") || 
		!strcmp(starting_line, "forum_id|person_id|joinDate\r\n") || 
		!strcmp(starting_line, "Forum.id|Person.id|joinDate\r\n")){
		return forum_hasMember_person;
		}
	if(	!strcmp(starting_line, "Comment.id|Post.id\n") || 
		!strcmp(starting_line, "comment_id|post_id\r\n") || 
		!strcmp(starting_line, "Comment.id|Post.id\r\n")){
		return comment_replyOf_post;
		}
	if(	!strcmp(starting_line, "comment_hasCreator_person\n") || 
		!strcmp(starting_line, "comment_id|person_id\r\n") || 
		!strcmp(starting_line, "comment_hasCreator_person\r\n")){
		return comment_hasCreator_person;
		}
	if(	!strcmp(starting_line, "Person.id|Organisation.id|workFrom\n") || 
		!strcmp(starting_line, "person_id|organisation_id|workFrom\r\n") || 
		!strcmp(starting_line, "Person.id|Organisation.id|workFrom\r\n")){
		return person_workAt_organisation;
		}
	if(	!strcmp(starting_line, "Person.id|Organisation.id|classYear\n") || 
		!strcmp(starting_line, "person_id|organisation_id|classYear\r\n") || 
		!strcmp(starting_line, "Person.id|Organisation.id|classYear\r\n")){
		return person_studyAt_organisation;
		}
	if(	!strcmp(starting_line, "Person.id|Post.id|creationDate\n") || 
		!strcmp(starting_line, "person_id|post_id|creationDate2\r\n") || 
		!strcmp(starting_line, "Person.id|Post.id|creationDate\r\n")){
		return person_likes_post;
		}
	if(	!strcmp(starting_line, "Person.id|Person.id\n") || 
		!strcmp(starting_line, "person_id1|person_id2\n") || 
		!strcmp(starting_line, "person_id1|person_id2\r\n") || 
		!strcmp(starting_line, "Person.id|Person.id\r\n")){
		return person_knows_person;
		}
	if(	!strcmp(starting_line, "Person.id|Place.id\n") || 
		!strcmp(starting_line, "person_id|place_id\r\n") || 
		!strcmp(starting_line, "Person.id|Place.id\r\n")){
		return person_isLocatedIn_place;
		}
	if(	!strcmp(starting_line, "Person.id|Tag.id\n") || 
		!strcmp(starting_line, "person_id|tag_id\r\n") || 
		!strcmp(starting_line, "Person.id|Tag.id\r\n")){
		return person_hasInterest_tag;
		}
	return invalid_relationship;
	}

node_type determine_node_type(char* starting_line, node_type graph_node_type){
	
	//Elegxos eisodou
	if(starting_line == NULL){
			printf("determine_relationship_type: Invalid input.");
			return invalid_node;
		}
	
	//Euresh typou
	if(	!strcmp(starting_line, "Post.id|Person.id\n") || 
		!strcmp(starting_line, "post_id|person_id\r\n") || 
		!strcmp(starting_line, "Post.id|Person.id\r\n")){
		return (graph_node_type==post) ? person : post;
		}
	if(	!strcmp(starting_line, "Forum.id|Post.id\n") || 
		!strcmp(starting_line, "forum_id|post_id\r\n") || 
		!strcmp(starting_line, "Forum.id|Post.id\r\n")){
		return (graph_node_type==forum) ? post : forum;
		}
	if(	!strcmp(starting_line, "Forum.id|Person.id|joinDate\n") || 
		!strcmp(starting_line, "forum_id|person_id|joinDate\r\n") || 
		!strcmp(starting_line, "Forum.id|Person.id|joinDate\r\n")){
		return (graph_node_type==forum) ? person : forum;
		}
	if(	!strcmp(starting_line, "Comment.id|Post.id\n") || 
		!strcmp(starting_line, "comment_id|post_id\r\n") || 
		!strcmp(starting_line, "Comment.id|Post.id\r\n")){
		return (graph_node_type==comment) ? post : comment;
		}
	if(	!strcmp(starting_line, "comment_hasCreator_person\n") || 
		!strcmp(starting_line, "comment_id|person_id\r\n") || 
		!strcmp(starting_line, "comment_hasCreator_person\r\n")){
		return (graph_node_type==comment) ? person : comment;
		}
	if(	!strcmp(starting_line, "Person.id|Organisation.id|workFrom\n") || 
		!strcmp(starting_line, "person_id|organisation_id|workFrom\r\n") || 
		!strcmp(starting_line, "Person.id|Organisation.id|workFrom\r\n")){
		return (graph_node_type==person) ? organisation : person;
		}
	if(	!strcmp(starting_line, "Person.id|Organisation.id|classYear\n") || 
		!strcmp(starting_line, "person_id|organisation_id|classYear\r\n") || 
		!strcmp(starting_line, "Person.id|Organisation.id|classYear\r\n")){
		return (graph_node_type==person) ? organisation : person;
		}
	if(	!strcmp(starting_line, "Person.id|Post.id|creationDate\n") || 
		!strcmp(starting_line, "person_id|post_id|creationDate2\r\n") || 
		!strcmp(starting_line, "Person.id|Post.id|creationDate\r\n")){
		return (graph_node_type==person) ? post : person;
		}
	if(	!strcmp(starting_line, "Person.id|Person.id\n") || 
		!strcmp(starting_line, "person_id1|person_id2\n") || 
		!strcmp(starting_line, "person_id1|person_id2\r\n") || 
		!strcmp(starting_line, "Person.id|Person.id\r\n")){
		return person;
		}
	if(	!strcmp(starting_line, "Person.id|Place.id\n") || 
		!strcmp(starting_line, "person_id|place_id\r\n") || 
		!strcmp(starting_line, "Person.id|Place.id\r\n")){
		return (graph_node_type==person) ? place : person;
		}
	if(	!strcmp(starting_line, "Person.id|Tag.id\n") || 
		!strcmp(starting_line, "person_id|tag_id\r\n") || 
		!strcmp(starting_line, "Person.id|Tag.id\r\n")){
		return (graph_node_type==person) ? tag : person;
		}
	
	printf("determine_node_type: Invalid type return\n");
	
	return invalid_node;
	}



//Utilities: Load Nodes
void load_person_csv(pam_graph* my_graph, char* filename){

	FILE* stream = fopen(filename, "r");
    char raw_line[1024];
	char* field;
	char* initial_line = "id|firstName|lastName|gender|birthday|creationDate|locationIP|browserUsed\n";
	char* initial_line2 = "id|firstName|lastName|gender|birthday|creationDate|locationIP|browserUsed\r\n";	//Sth dosmenh main yparxei ayth h arxikh seira
	int num_of_attributes = 7; //To id den metrietai
	int temp_id;
	int i;
	pam_properties* temp_properties;
	pam_node* temp_node;
	
	//1. Elegxos an exoume anoi3ei to arxeio kai an einai to swsto
	if(stream == NULL){
			printf("load_person_csv: Error opening file.\n");
			return;
		}
	if(!fgets(raw_line, 1024, stream)){
			printf("load_person_csv: Invalid filename.\n");
			return;
		}
	if(strcmp(initial_line, raw_line) && strcmp(initial_line2, raw_line)){
		printf("load_person_csv: Error: Invalid file error.\n");
		return;
		}
	
	//2.Diavase ka8e grammh kai pros8ese thn sto grafo
    while (fgets(raw_line, 1024, stream)){
		//printf("%s\n"raw_line);
		//Dhmiourghse th domh me tis idiothtes
		if( (temp_properties = pam_createProperties(num_of_attributes)) == NULL ){
			printf("load_person_csv: Unable to allocate memory\n");
			return;
			}
			
		//1. id
		field = strtok(raw_line, "|");
		temp_id = atoi(field);
		
		//firstName | lastName | gender | birthday | creationDate | locationIP | browserUsed
		for(i=0; i<num_of_attributes; i++){
			if( (field = strtok(NULL, "|")) != NULL){
				pam_setStringProperty(field, i, temp_properties);
				//printf("%s\t", field);
				}
			else{
				pam_setStringProperty("(no data)", i, temp_properties);
				}
			}
		
		//Dhmiourghse ton komvo
		temp_node = pam_createNode(temp_id, temp_properties);
		if(temp_node == NULL){
			printf("load_person_csv: Unable to create Node\n");
			return;
			}
		
		//Pros8ese ton komvo
		if(!pam_insertNode(my_graph, temp_node)){
			printf("load_person_csv: Unable to insert Node to Graph\n");
			return;
			}
		}
		
	//3. Meta to peras ths diadikasias, kleisimo tou arxeiou
	fclose(stream);
	}

void load_post_csv(pam_graph* my_graph, char* filename){
	
	FILE* stream = fopen(filename, "r");
    char raw_line[1024];
	char* field;
	char* initial_line = "id|imageFile|creationDate|locationIP|browserUsed|language|content\n";
	char* initial_line2 = "id|imageFile|creationDate|locationIP|browserUsed|language|content\r\n";
	int num_of_attributes = 6; //To id den metrietai
	int temp_id;
	int counter;
	int i, j;
	pam_properties* temp_properties;
	pam_node* temp_node;
	
	//1. Elegxos an exoume anoi3ei to arxeio kai an einai to swsto
	if(stream == NULL){
			printf("load_post_csv: Error opening file.\n");
			return;
		}
	if(!fgets(raw_line, 1024, stream)){
			printf("load_post_csv: Invalid filename.\n");
			return;
		}
	if(strcmp(initial_line, raw_line) && strcmp(initial_line2, raw_line)){
		printf("load_post_csv: Error: Invalid file error.\n");
		return;
		}
	
	//2.Diavase ka8e grammh kai pros8ese thn sto grafo
    while (fgets(raw_line, 1024, stream)){
		//printf("%s\n"raw_line);
		//Dhmiourghse th domh me tis idiothtes
		if( (temp_properties = pam_createProperties(num_of_attributes)) == NULL ){
			printf("load_post_csv: Unable to allocate memory\n");
			return;
			}

		//1. id
		field = strtok(raw_line, "|");
		temp_id = atoi(field);
		
		//imageFile|creationDate|locationIP|browserUsed|language|content
		for(j=0; j<num_of_attributes; j++){
			if( (field = strtok(NULL, "|")) != NULL){
				pam_setStringProperty(field, j, temp_properties);
				//printf("%s\t", field);
				}
			else{
				pam_setIntegerProperty(-1, j, temp_properties);
				}
			}
		
		//Dhmiourghse ton komvo
		temp_node = pam_createNode(temp_id, temp_properties);
		if(temp_node == NULL){
			printf("load_post_csv: Unable to create Node\n");
			return;
			}
		
		//Pros8ese ton komvo
		if(!pam_insertNode(my_graph, temp_node)){
			printf("load_post_csv: Unable to insert Node to Graph\n");
			return;
			}
		}
		
	//3. Meta to peras ths diadikasias, kleisimo tou arxeiou
	fclose(stream);
	}

void load_forum_csv(pam_graph* my_graph, char* filename){
	
	FILE* stream = fopen(filename, "r");
    char raw_line[1024];
	char* field;
	char* initial_line = "id|title|creationDate\n";
	char* initial_line2 = "id|title|creationDate\n";
	int num_of_attributes = 2; //To id den metrietai
	int temp_id;
	pam_properties* temp_properties;
	pam_node* temp_node;
	
	//1. Elegxos an exoume anoi3ei to arxeio kai an einai to swsto
	if(stream == NULL){
			printf("load_forum_csv: Error opening file.\n");
			return;
		}
	if(!fgets(raw_line, 1024, stream)){
			printf("load_forum_csv: Invalid filename.\n");
			return;
		}
	if(strcmp(initial_line, raw_line) && strcmp(initial_line2, raw_line)){
		printf("load_forum_csv: Error: Invalid file error.\n");
		return;
		}
	
	//2.Diavase ka8e grammh kai pros8ese thn sto grafo
    while (fgets(raw_line, 1024, stream)){
		//printf("%s\n"raw_line);
		//Dhmiourghse th domh me tis idiothtes
		if( (temp_properties = pam_createProperties(num_of_attributes)) == NULL ){
			printf("load_forum_csv: Unable to allocate memory\n");
			return;
			}
			
		//1. id
		field = strtok(raw_line, "|");
		temp_id = atoi(field);
		
		//2. title
		field = strtok(NULL, "|");
		pam_setStringProperty(field, 0, temp_properties);
		
		//3. creationDate
		field = strtok(NULL, "|");
		pam_setStringProperty(field, 1, temp_properties);
		
		
		//Dhmiourghse ton komvo
		temp_node = pam_createNode(temp_id, temp_properties);
		if(temp_node == NULL){
			printf("load_forum_csv: Unable to create Node\n");
			return;
			}
		
		//Pros8ese ton komvo
		if(!pam_insertNode(my_graph, temp_node)){
			printf("load_forum_csv: Unable to insert Node to Graph\n");
			return;
			}
		}
		
	//3. Meta to peras ths diadikasias, kleisimo tou arxeiou
	fclose(stream);
	}

void load_tag_csv(pam_graph* my_graph, char* filename){
	
	FILE *infile;
	char firstline[30], tag_name[40], url[100], stringID[10];
	char* initial_line = "id|name|url\n";
	char* initial_line2 = "id|name|url\n";
	int tag_id, num_of_attributes=2;
	pam_properties* temp_properties;
	pam_node* temp_node;
	
	//	elegxos dedomenwn
	if ((infile = fopen("tag.csv", "r")) == NULL)
	{	printf("Cannot open tags.csv\n");
		return;
		}
	fscanf(infile, "%s", firstline);
	
	if(strcmp(initial_line, firstline) && strcmp(initial_line2, firstline))
	{	printf("load_tag_csv: Wrong input file\n");
		return;
	}
	
	while( fscanf(infile, "%[^|]|%[^|]|%[^'\n']", stringID, tag_name, url) >= 3)
	{	tag_id = atoi(stringID);
		// dhmiourgia properties
		if( (temp_properties = pam_createProperties(num_of_attributes)) == NULL )
		{	printf("load_tag_csv: Unable to allocate memory\n");
			return;
		}
		pam_setStringProperty(tag_name, 0, temp_properties);
		pam_setStringProperty(url, 1, temp_properties);
	
		//Dhmiourghse ton komvo
		temp_node = pam_createNode(tag_id, temp_properties);
		if(temp_node == NULL){
			printf("load_tag_csv: Unable to create Node\n");
			return;
			}
		
		//Pros8ese ton komvo
		if(!pam_insertNode(my_graph, temp_node)){
			printf("load_tag_csv: Unable to insert Node to Graph\n");
			return;
			}
	}
	fclose(infile);
	}



//Utilities: Load Edges
void load_relationship_csv(pam_graph* my_graph, char* filename, char mode){
	int i, temp_id, current_line;
	FILE* stream = fopen(filename, "r");
    char raw_line[1024];
	char* field;
	pam_properties* temp_properties;
	pam_edge* temp_edge;
	
	property_type* properties;
	bool has_id;
	node_type my_node_type;
	int num_of_attributes;	//E3aireitai to id
	int starting_id, ending_id = -1;
	relationship_type my_relationship_type;
	
	//1. Elegxos an exoume anoi3ei to arxeio
	//printf("load_relationship_csv: 1. \n");
	if(stream == NULL){
			printf("load_relationship_csv: Error opening file.\n");
			return;
		}
	if(!fgets(raw_line, 1024, stream)){
			printf("load_relationship_csv: Invalid filename.\n");
			return;
		}
	
	//2. Eyresh typou sysxetishs
	//printf("load_relationship_csv: 2. \n");
	my_node_type = determine_node_type(raw_line, my_graph->my_node_type);
	my_relationship_type = determine_relationship_type(raw_line);
	
	//3. Analoga ton typo, ypologise to eidos twn Properties
	//printf("load_relationship_csv: 3. \n");
	switch(my_relationship_type){
		
		case post_hasCreator_person ... person_hasInterest_tag:
			//Mono id
			has_id = true;
			num_of_attributes = 0;
			properties = NULL;
			break;
			
		case forum_hasMember_person ... person_likes_post:
			//id kai data
			has_id = true;
			num_of_attributes = 1;
			if( (properties = malloc(num_of_attributes * sizeof(property_type))) == NULL){
				return;
				}
			properties[0] = string_type;
			break;
		
		case person_speaks_language ... person_email_emailaddress:
			//Mono data
			has_id = false;
			num_of_attributes = 1;
			if( (properties = malloc(num_of_attributes * sizeof(property_type))) == NULL){
				return;
				}
			properties[0] = string_type;
			break;
		
		default:
			printf("load_relationship_csv: Unknown file format\n");
			return;
		}
	
	//4. Vasei tou typou, diavase ka8e grammh kai pros8ese thn sto grafo
	//printf("load_relationship_csv: 4. \n");
	current_line = 0;
    while (fgets(raw_line, 1024, stream)){
		current_line++;
		
		//printf("load_relationship_csv: %s\n", raw_line);
		//1. starting id
		field = strtok(raw_line, "|\n");
		if(field == NULL){
			printf("load_relationship_csv: File format error in line %d\n", current_line);
			break;
			}
		starting_id = atoi(field);
		
		if(has_id){
			//2. ending id
			field = strtok(NULL, "|\r\n");
			ending_id = atoi(field);
			}
		
		// add edge the other way around
		if (mode == 1)
		{	temp_id = ending_id;
			ending_id = starting_id;
			starting_id = temp_id;
		}
		
		//Dhmiourghse th domh me tis idiothtes
		if( (temp_properties = pam_createProperties(num_of_attributes)) == NULL ){
			printf("load_relationship_csv: Unable to allocate memory\n");
			free(properties);
			return;
			}
		
		field = strtok(NULL, "|\r\n");
		for(i=0; i<num_of_attributes && field != NULL; i++){
			
			if(properties[i] == int_type){
				pam_setIntegerProperty(atoi(field), i, temp_properties);
				}
			else{
				pam_setStringProperty(field, i, temp_properties);
				}
			field = strtok(NULL, "|\r\n");
			}

		//Dhmiourghse thn akmh
		temp_edge = pam_createEdge(starting_id, ending_id, temp_properties, my_node_type, my_relationship_type);
		if(temp_edge == NULL){
			printf("load_relationship_csv: Unable to create Edge\n");
			free(properties);
			return;
			}
		
		//Pros8ese thn akmh
		//printf("INSERTING\n");
		pam_insertEdge(my_graph, temp_edge);
		}
		
	//5. Meta to peras ths diadikasias, kleisimo tou arxeiou
	//printf("load_relationship_csv: 5. \n");
	fclose(stream);
	}
