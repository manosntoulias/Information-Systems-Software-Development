#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "pam_graph.h"
#include "defines.h"
#include "topN.h"

#define BUFF_SIZE 1024

//mode for thread "count members". 0, counts all members of forums.
// 1, insterts the member in a topN graph
char mode;

//global tables
forum_size *f_sizes;
int **member_ids;
int *last_elem;
pam_graph **top_graphs;
pam_graph *main_graph;

int memory_offset;
pthread_mutex_t *mutex;
char personknows[] = "person_knows_person.csv";
char personcsv[] = "person.csv";
char forums[] = "forum.csv";
char forum_has_member[] = "forum_hasMember_person.csv";
int lines_per_thread, modulo, global_forums_num;
long int *last_offset;

int compare(void* arg1, void *arg2){
	int integer1[1], integer2[1]; 
	memcpy(integer1, arg1+memory_offset, sizeof(int));
	memcpy(integer2, arg2+memory_offset, sizeof(int));
	return (*integer2 - *integer1); //descending order
}

int count_lines(char *filename) {
	// find the number of lines in a file
	FILE *infile;
	char buffer[BUFF_SIZE];
	int lines=0;
	
	if ((infile = fopen(filename, "r")) == NULL)
	{	printf("count_lines: Cannot open file <%s>\n", filename);
		return -1;
	}
	// drop the first line
	fscanf(infile, "%[^\n]\n", buffer);
	
	while (fscanf(infile, "%[^\n]\n", buffer) == 1)
	{	lines++;
	}
	fclose(infile);
	return lines;
}

bool create_partitions(char *filename, long int *thread_offset, int num_of_threads) {
	//open file to create partitions of file for each thread
	int line=0, j=0;
	FILE *infile;
	char buffer[BUFF_SIZE];
	
	if ((infile = fopen(filename, "r")) == NULL)
	{	printf("create_partitions: Cannot open forum_has_member.csv\n");
		return false;
	}
	
	// drop first line
	fscanf(infile, "%[^\n]\n", buffer);
	
	thread_offset[j++] = ftell(infile);
	while (fscanf(infile, "%[^\n]\n", buffer) == 1)
	{	line++;
		if (line % lines_per_thread == 0 && j < num_of_threads)
		{	if ((thread_offset[j] = ftell(infile)) == -1)
			{	perror("create_partitions/ftell:");
				return false;
			}
			j++;
		}
	}
	fclose(infile);
	return true;
}

pam_properties* setPersonProperties(char *fn, char *ln, char *gender, char *birthdate, char *creationdate, char *ip, char *browser) {
	pam_properties* prop;
	prop = pam_createProperties(7);
	pam_setStringProperty(fn, 0, prop);
	pam_setStringProperty(ln, 1, prop);
	pam_setStringProperty(gender, 2, prop);
	pam_setStringProperty(birthdate, 3, prop);
	pam_setStringProperty(creationdate, 4, prop);
	pam_setStringProperty(ip, 5, prop);
	pam_setStringProperty(browser, 6, prop);
	
	return prop;
}


void *count_members(void *arg) {
	long int *offset = (long int*)arg;
	FILE *infile;
	char stringID[20], memberID[20];
	char buffer[BUFF_SIZE];
	int id, member_id, local_lines_per_thread, line=0, start, end, found, middle;
	
	// open a part of file "forums has member" to count how many members forums in this part have
	if ((infile = fopen(forum_has_member, "r")) == NULL)
	{	printf("count_members: Cannot open forum_has_member.csv\n");
		return NULL;
	}
	if (fscanf(infile, "%[^|]|%[^|]|%[^\n]\n", buffer, buffer, buffer) != 3)
	{	printf("count_members: Wrong input file\n");
		return NULL;
	}

	
	local_lines_per_thread = lines_per_thread;
	// this part of file
	if (fseek ( infile, *offset, SEEK_SET))
	{	perror("count_members/fseek");
		return NULL;
	}
	if (offset == last_offset)
	{	local_lines_per_thread += modulo;
	}
	
	while (line < local_lines_per_thread)
	{	if (fscanf(infile, "%[^|]|%[^|]|%[^\n]\n", stringID, memberID, buffer) != 3)
		{	break;
		}
		
		id = atoi(stringID);
		
		found = -1;
		// binary search
		start = 0; end = global_forums_num - 1;
		if (f_sizes[start].id == id)
		{	found = start;
		}
		else if (f_sizes[end].id == id)
		{	found = end;
		}
		else
		{	while (end - start > 1)
			{	middle = (start + end)/2;
				if (f_sizes[middle].id == id)
				{	found = middle;
					break;
				}
				else if (f_sizes[middle].id < id)
					end = middle;
				else
					start = middle;
			}
		}
		
		if (found != -1)
		{
			if (mode == 1)
			{	member_id = atoi(memberID);
			}
		
			//CRITICAL SECTION
			if (pthread_mutex_lock(mutex))
			{	perror("thread_counter/mutex_lock");
				break;
			}
			if (mode == 0)
				f_sizes[found].size++;
			else
			{	member_ids[found][last_elem[found]] = member_id;
				last_elem[found]++;
			}
		
			if (pthread_mutex_unlock(mutex))
			{	perror("thread_counter/mutex_unlock");
				break;
			}
			//END OF CRITICAL
		}
		line++;
	}
	fclose(infile);
	
	return;
}

void *insert_top_members(void *arg) {
	long int *offset = (long int*)arg;
	FILE *infile;
	char fn[20], ln[20], gender[7], birthday[11],creationdate[25], ip[20], browser[15];
	char buffer[BUFF_SIZE], stringID[20], memberID[20];
	int *found, id, member_id, local_lines_per_thread, line=0, start, end, middle, i;
	pam_properties *prop;
	pam_node *node;
	
	// open a part of file "person.csv" to insert each member to its forum graph
	if ((infile = fopen(personcsv, "r")) == NULL)
	{	printf("insert_top_members: Cannot open person.csv\n");
		return NULL;
	}
	if (fscanf(infile, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", buffer, buffer, buffer, buffer, buffer, buffer, buffer, buffer) != 8)
	{	printf("insert_top_members: Wrong input file\n");
		return NULL;
	}
	
	local_lines_per_thread = lines_per_thread;
	// this part of file
	if (fseek ( infile, *offset, SEEK_SET))
	{	perror("count_members/fseek");
		return NULL;
	}
	if (offset == last_offset)
	{	local_lines_per_thread += modulo;
	}
	
	char first, no_properties=0, rederino; 
	while (line < local_lines_per_thread)
	{	if ((rederino = fscanf(infile, " %[^|]| %[^|]| %[^|]| %[^|]| %[^|]| %[^|]| %[^|]| %[^\n]\n", memberID, fn, ln, gender, birthday, creationdate, ip, browser)) != 8)
		{	fscanf(infile, "%s\n", buffer);
			if (rederino > 0) // line that at least has id
			{	if (no_properties == 0)
				{	printf("Warning: About to add nodes with missing(NULL) properties to the graphs..\n");
					no_properties = 1;
				}
				strcpy(fn, "");
				strcpy(ln, "");
				strcpy(gender, "");
				strcpy(birthday, "");
				strcpy(creationdate, "");
				strcpy(ip, "");
				strcpy(browser, "");
			}
			else // corrupted line
			{	line++;
				continue;
			}
			//printf("%d %s\n", rederino, buffer);
		}

		member_id = atoi(memberID);
		first = 1;
		//printf("member has id: %d\n", member_id);
		for (i=0; i < global_forums_num; i++)
		{
			// binary search
			found = (int*)bsearch(&member_id, member_ids[i], last_elem[i], sizeof(int), compare);

			
		
			if (found != NULL)
			{	
				if (first == 1)
				{	prop = setPersonProperties(fn, ln, gender, birthday, creationdate, ip, browser);
					node = pam_createNode(member_id, prop);
					first = 0;
				}
		
				//CRITICAL SECTION
				if (pthread_mutex_lock(mutex))
				{	perror("insert_top_members/mutex_lock");
					break;
				}
				
				if (pam_insertNode(top_graphs[i], node) == false)
				{	printf("by insert_top_members\n");
					break;
				}
				if (pam_lookupNode(main_graph, node->id) == NULL)
				{	if (pam_insertNode(main_graph, node) == false)
					{	printf("by insert_top_members(main_graph)\n");
						break;
					}
				}
			
				if (pthread_mutex_unlock(mutex))
				{	perror("insert_top_members/mutex_unlock");
					break;
				}
				//END OF CRITICAL
			}
		}
		line++;
	}
	fclose(infile);
	
	return;
}


void *insert_top_edges(void *arg) {
	long int *offset = (long int*)arg;
	FILE *infile;
	char fn[20], ln[20], gender[7], birthday[11],creationdate[25], ip[20], browser[15];
	char buffer[BUFF_SIZE], stringID[20], stringID2[20];
	int *found, start_id, end_id, local_lines_per_thread, line=0, start, end, middle, i;
	pam_properties *prop;
	pam_node *node;
	pam_edge *edge;
	
	// open a part of file "person_knows_person.csv" to insert each member to its forum graph
	if ((infile = fopen(personknows, "r")) == NULL)
	{	printf("insert_top_edges: Cannot open person.csv\n");
		return NULL;
	}
	if (fscanf(infile, "%[^|]|%[^\n]\n", buffer, buffer) != 2)
	{	printf("insert_top_edges: Wrong input file\n");
		return NULL;
	}
	
	local_lines_per_thread = lines_per_thread;
	// this part of file
	if (fseek ( infile, *offset, SEEK_SET))
	{	perror("count_members/fseek");
		return NULL;
	}
	if (offset == last_offset)
	{	local_lines_per_thread += modulo;
	}
	int cn=0, cn2=0;
	char first; 
	while (line < local_lines_per_thread)
	{	if (fscanf(infile, "%[^|]|%[^\n]\n", stringID, stringID2) != 2)
		{	break;
		}

		if (strcmp(stringID2, "0") == 0)
			cn++;
		
		start_id = atoi(stringID);
		end_id = atoi(stringID2);
		
		for (i=0; i < global_forums_num; i++)
		{	
			//search if both verticles of edge belong in graph
			if ((pam_lookupNode(top_graphs[i], start_id)) == NULL)
				continue;
			if ((pam_lookupNode(top_graphs[i], end_id)) == NULL)
				continue;
			
			prop = setPkP_Properties();
			edge = pam_createEdge(start_id, end_id, prop, person, person_knows_person);
			
					
			//CRITICAL SECTION
			if (pthread_mutex_lock(mutex))
			{	perror("insert_top_edges/mutex_lock");
				break;
			}
			
			if (pam_insertEdge(top_graphs[i], edge) == false)
			{	printf("by insert_top_edges\n");
				break;
			}
		
			if (pthread_mutex_unlock(mutex))
			{	perror("insert_top_edges/mutex_unlock");
				break;
			}
			//END OF CRITICAL
			
		}
		line++;
	}
	fclose(infile);
	
	return;
}

pam_forum *topN(int N, pam_graph** main_g, pam_graph*** top_g) {
	FILE *infile;
	int num_of_forums, forum_id, i=0, j=0, read, line_counter;
	char buffer[BUFF_SIZE], stringID[20];
	int num_of_threads = NUMBER_OF_THREADS;
	long int thread_offset[num_of_threads];
	
	// open forums.csv to find the number of forums
	if ((num_of_forums = count_lines(forums)) == -1)
	{	printf("by topN\n");
		return NULL;
	}
	//printf("there are %d forums\n", num_of_forums);
	
	// desmeush mnhmhs gia id kai megethos kathe forum
	if ((f_sizes = calloc(num_of_forums, sizeof(forum_size))) == NULL)
	{	printf("topN: Cannot allocate memory for forum id and size\n");
		return NULL;
	}
	
	
	
	// open forums.csv to find the id of each forum
	if ((infile = fopen(forums, "r")) == NULL)
	{	printf("topN: Cannot open forums.csv\n");
		return NULL;
	}
	if (fscanf(infile, "%[^|]|%[^|]|%[^\n]\n", stringID, buffer, buffer) != 3)
	{	printf("topN: Wrong input file\n");
		return NULL;
	}
	
	while (fscanf(infile, "%[^|]|%[^|]|%[^\n]\n", stringID, buffer, buffer) == 3)
	{	f_sizes[i].id = atoi(stringID);
		//printf("id is %d\n", f_sizes[i].id);
		i++;
	}
	fclose(infile);
	
	
	//open forum_has_member.csv to count the number of lines
	if ((line_counter = count_lines(forum_has_member)) == -1)
	{	printf("by topN\n");
		return NULL;
	}
	
	//set global data
	lines_per_thread = line_counter/num_of_threads;
	modulo = line_counter % num_of_threads;
	
	pthread_t thread[num_of_threads];
	last_offset = thread_offset + num_of_threads -1;
	global_forums_num = num_of_forums;
	//printf("\nline per threads: %d\n", lines_per_thread);
	
	i=0;
	//open forum_has_member to create partitions of file for each thread
	if (create_partitions(forum_has_member, thread_offset, num_of_threads) == false)
	{	printf("by topN\n");
		return NULL;
	}
		
	// sort the IDs of the forums for binary search in the threads
	memory_offset = sizeof(int);
	qsort(f_sizes, (size_t)num_of_forums, sizeof(forum_size), compare);
		
	//desmeush mnhmhs mutex
	if ((mutex = malloc(sizeof(pthread_mutex_t))) == NULL)
	{	printf("Cannot allocate memory for mutex\n");
		return NULL;
	}
	
	if (pthread_mutex_init(mutex, NULL) != 0)
	{	perror("topN/mutex_destroy");
		return NULL;
	}
	
	//threads to count the members of each forum
	mode = 0;
	
	for (i=0; i< num_of_threads; i++)
		if (pthread_create(thread+i, NULL, count_members, thread_offset + i))
			{	perror("topN: error while creating threads -");
				return NULL;
			}
	
	for (i=0; i <  num_of_threads; i++)
		if(pthread_join(thread[i], NULL))
		{	perror("topN: error while joining threads -");
			return NULL;
		}
	
	
	// sort the sizes of the forums
	memory_offset = 0;
	qsort(f_sizes, (size_t)num_of_forums, sizeof(forum_size), compare);
	

	
	//Keep the topN forum sizes
	if ((f_sizes = realloc(f_sizes, N*sizeof(forum_size))) == NULL)
	{	free(f_sizes);
		printf("topN: Cannot reallocate memory for forum ids and sizes\n");
		return NULL;
	}
	global_forums_num = N;
	
	
	
	//find the names of the topN forums
	pam_forum *my_forums;
	char name[70];
	
	if ((my_forums = malloc(N*sizeof(pam_forum))) == NULL)
	{	free(f_sizes);
		printf("topN: Cannot allocate memory for pam_forum\n");
		return NULL;
	}
	
	if ((infile = fopen(forums, "r")) == NULL)
	{	printf("topN: Cannot open forums.csv to find names\n");
		return NULL;
	}
	if (fscanf(infile, "%[^|]|%[^|]|%[^\n]\n", stringID, buffer, buffer) != 3)
	{	printf("topN: Wrong input file\n");
		return NULL;
	}
	
	while (fscanf(infile, "%[^|]|%[^|]|%[^\n]\n", stringID, name, buffer) == 3)
	{	forum_id = atoi(stringID);
		for (i=0; i<N; i++)
		{	if (forum_id == f_sizes[i].id)
			{	my_forums[i].size = f_sizes[i].size;
				strcpy(my_forums[i].name, name);
			}
		}
		
	}
	fclose(infile);
	
	
	// sort again the IDs of the forums for binary search in the threads
	memory_offset = sizeof(int);
	qsort(f_sizes, (size_t)global_forums_num, sizeof(forum_size), compare);
	
	
	//create topN graphs
	if ((top_graphs = malloc(global_forums_num*sizeof(pam_graph*))) == NULL)
	{	printf("topN: Cannot allocate memory for top_graphs\n");
		return NULL;
	}
	if ((main_graph = pam_createGraph(4, 50, person)) == NULL)
	{	printf("topN: Cannot allocate memory for main_graph\n");
		return NULL;
	}
	//create lists with the ids of every member
	if ((member_ids = malloc(global_forums_num*sizeof(int*))) == NULL)
	{	printf("topN: Cannot allocate memory for member_ids\n");
		return NULL;
	}
	//points the end for each member_id list
	if ((last_elem = calloc(global_forums_num, sizeof(int))) == NULL)
	{	printf("topN: Cannot allocate memory for last_elem\n");
		return NULL;
	}
	for (i=0; i<global_forums_num; i++)
	{	if ((top_graphs[i] = pam_createGraph(4, 20, person)) == NULL)
		{	printf("in CreateGraph by topN\n");
			return NULL;
		}
		if ((member_ids[i] = malloc(f_sizes[i].size*sizeof(int))) == NULL)
		{	printf("topN: Cannot allocate memory for member_ids\n");
			return NULL;
		}
	}	
	
	//Threads to find the ids of members for each topN forum/graphs
	mode = 1;
	
	for (i=0; i< num_of_threads; i++)
		if (pthread_create(thread+i, NULL, count_members, thread_offset + i))
			{	perror("topN: error while creating threads -");
				return NULL;
			}
	
	for (i=0; i <  num_of_threads; i++)
		if(pthread_join(thread[i], NULL))
		{	perror("topN: error while joining threads -");
			return NULL;
		}
		
		
	//for (i=0; i<global_forums_num; i++)
		//printf("person id %d size %d\n", member_ids[0][i], last_elem[i]);
	
	//sort member ids
	memory_offset = 0;
	for (i=0; i < global_forums_num; i++)
		qsort(member_ids[i], (size_t)last_elem[i], sizeof(int), compare);
		
	//count lines for person.csv
	if ((line_counter = count_lines(personcsv)) == -1)
	{	printf("by topN\n");
		return NULL;
	}
	//printf("there are---%d--- persons\n", line_counter);
	
	//set global data
	lines_per_thread = line_counter/num_of_threads;
	modulo = line_counter % num_of_threads;
	
	//printf("there are---%d--- persons| lines per thread %d\n", line_counter, lines_per_thread);
	
	if (create_partitions(personcsv, thread_offset, num_of_threads) == false)
	{	printf("by topN\n");
		return NULL;
	}
	
	// threads that insert the members of topN graphs
	for (i=0; i< num_of_threads; i++)
		if (pthread_create(thread+i, NULL, insert_top_members, thread_offset + i))
			{	perror("topN: error while creating threads -");
				return NULL;
			}
	
	for (i=0; i <  num_of_threads; i++)
		if(pthread_join(thread[i], NULL))
		{	perror("topN: error while joining threads -");
			return NULL;
		}
	
	free(last_elem);
	for (i=0; i<N; i++)
		free(member_ids[i]);
	free(member_ids);
	free(f_sizes);
	
	//count lines for personknowperson.csv
	if ((line_counter = count_lines(personknows)) == -1)
	{	printf("by topN\n");
		return NULL;
	}

	
	//set global data
	lines_per_thread = line_counter/num_of_threads;
	modulo = line_counter % num_of_threads;
	
	if (create_partitions(personknows, thread_offset, num_of_threads) == false)
	{	printf("by topN\n");
		return NULL;
	}
		
	// threads that insert the edges of topN graphs
	for (i=0; i< num_of_threads; i++)
		if (pthread_create(thread+i, NULL, insert_top_edges, thread_offset + i))
			{	perror("topN: error while creating threads -");
				return NULL;
			}
	
	for (i=0; i <  num_of_threads; i++)
		if(pthread_join(thread[i], NULL))
		{	perror("topN: error while joining threads -");
			return NULL;
		}
		
	if (pthread_mutex_destroy(mutex) != 0)
	{	perror("topN/mutex_destroy");
		return NULL;
	}
	
	free(mutex);
	
	pam_graph *temp_g;
	// sort the topN graphs base on size
	for (j=0; j<N-1; j++)
	{	for (i=0; i<N-j-1; i++)
		{	if (top_graphs[i]->num_of_nodes < top_graphs[i+1]->num_of_nodes)
			{	temp_g = top_graphs[i];
				top_graphs[i] = top_graphs[i+1];
				top_graphs[i+1] = temp_g;
			}
		}
	}
	
	*main_g = main_graph;
	*top_g = top_graphs;
	
	return my_forums;
}


/*
int main(void)
{	int N=6;
	pam_forum *my_forums;
	pam_graph *main_g, **top_g;
	int i;
	
	
	if ((my_forums = topN(N, &main_g, &top_g)) != NULL)
	{	
	
		printf("\n\n");
		printf("\n\n");
		
		for (i=0; i < global_forums_num; i++)
		{	printf("graph_name %s has %d members\n", my_forums[i].name, my_forums[i].size);
			pam_destroyliteGraph(top_graphs[i], 0);
		}
		printf("graph main has %d members\n", main_g->num_of_nodes);
		pam_destroyGraph(main_g);
		free(top_g);
		free(my_forums);
	
		return 0;
	}
	else
		return 1;
}
*/