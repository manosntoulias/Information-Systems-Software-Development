#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h> 
#include <limits.h>
#include <float.h>
#include "defines.h"
#include "pam_graph.h"
#include "pam_utilities.h"
#include "community_algorithms.h"
#include "topN.h"

int main(void){
	pam_graph* my_graph, *main_graph, **topN_graphs;
	Communities* my_c;
	pam_forum *my_forums;
	int N=6, i, j, k;
	int initial_hashtable_size = DEFAULT_HASHTABLE_SIZE;
	int num_of_cells = DEFAULT_NUM_OF_CELLS;
	
	//1. Dhmiourgia arxikou grafou (person knows person)
	printf("1. Create Graph\n");
	my_graph = pam_createGraph(initial_hashtable_size, num_of_cells, person);
	printf("2. Load Nodes\n");
	load_person_csv(my_graph, "person.csv");
	printf("3. Load Edges\n");
	load_relationship_csv(my_graph, "person_knows_person.csv", 0);
	
	//TopN forums
	printf("4. find topN forums\n");
	
	if ((my_forums = topN(N, &main_graph, &topN_graphs)) == NULL)
	{	printf("by main\n");
		return -1;
	}
	
	for (i=0; i<N; i++)
	{	printf("%d. forum <%s> has %d members\n", i+1, my_forums[i].name, my_forums[i].size);
	}
	
	
	//Community algorithms
	printf("5. CPM\n");
	
	//my_c = cliquePercolationMethod(3, my_graph);
	
	
	printf("6. GN\n");
	
	gn_Communities *gn_cmm;
	FILE *outfile;
	
	if ((outfile = fopen("GN_results.txt", "w")) == NULL)
	{	printf("main: Cannot create <GN_results.txt>\n");
		return -1;
	}
	
	for (i=0; i<N; i++)
	{	fprintf(outfile, "\n\n--- graph %d --- %d members ---\n", i+1, topN_graphs[i]->num_of_nodes);
		gn_cmm = Girvan_Newman(topN_graphs[i], 9000.0);
		for (j=0; j < gn_cmm->size; j++ )
		{	fprintf(outfile, "\ncommunity: ", j, gn_cmm->data[j].size);
			for (k=0; k < gn_cmm->data[j].size; k++)
				fprintf(outfile, "%d ", gn_cmm->data[j].data[k]);
		}
		
		for (i=0; i < gn_cmm->size; i++)
			free(gn_cmm->data[i].data);
		free(gn_cmm->data);
		free(gn_cmm);
	}
	fprintf(outfile, "\n");
	fclose(outfile);
	printf("GN results have been stored into file <GN_results.txt>");
	
	
	
	//Diagrafh grafwn
	printf("\n7. delete graphs\n");
	
	for (i=0; i<N; i++)
		pam_destroyliteGraph(topN_graphs[i], 0);
	pam_destroyGraph(main_graph);
	free(topN_graphs);
	free(my_forums);
	
	pam_destroyGraph(my_graph);
	
	return 0;
	}
	
/*
 =============================================================================
  Name      : main.c
  Version   : 1
  Copyright : pam-Team
 ============================================================================
*/

/*
int number_of_top_n_forums
Forum** getTopNForums();
boolean validateTopNResults(Forum **forums);
boolean validateCPMResults(Cpm **cpm);
boolean validateGNResults(Gn **gn);
void free_memory(Forum **topForums, Cpm **cpm, Gn **gn);

int compareForums(const void *data1, const void *data2){
	forum_data *forum1 = (forum_data *)data1;
	forum_data *forum2 = (forum_data *)data2;

	int nameCompare = strcmp(forum1->name, forum2->name);
	int idCompare = forum1->id - forum2->id;
	int sizeCompare = forum1->size - forum2->size;

	if(nameCompare != 0)
		return 1;
	if (idCompare != 0)
		return 1;
	if (sizeCompare != 0)
		return 1;
	
	return 0;
	}

int compareIntegers(const void *data1, const void *data2){
	return (*(int *)data1 - *(int *)data2);
	}

	
int main(void){
	Communities *cpm = NULL;
	Communities *gn = NULL;
	printf("\n");
	boolean error = False;

	//Create graph
	int bucketsNumber = 10;
	int bucketSize = 5;
	pam_graph* graph = pam_createGraph(bucketsNumber, bucketSize, person);

	//Compute the top-6 forums
	forum_data **topForums = topN(NUMBER_OF_TOP_N_FORUMS);

	//Validate top-6 results
	if(validateTopNResults(topForums) == False)
		error = True;

	int cliqueSize[2] = {3, 4};

	if(!error){
		//Compute results with CPM algorithm for size=3,4
		cpm = cliquePercolationMethod(graph, cliqueSize);

		//Validate the results of CPM algorithm
		if(validateCPMResults(cpm) == False)
			error = True;
			}

	if(!error){
		//Compute results with GN algorithm
		gn = computeGNResults(graph, DBL_MAX);

		//Validate the results of GN algorithm
		if(validateGNResults(gn) == False)
			error = True;
		}

	if(!error)
		printf("Successfully passed the test!!!\n");

	printf("\n\n");
	free_memory(topForums, cpm, gn);
	return EXIT_SUCCESS;
	} 


void free_memory(Forum **topForums, Cpm **cpm, Gn **gn) {

        int i, j;
        if(topForums != NULL){
                for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i)
                        free(topForums[i]->name);
                free(topForums);
        }

	if(gn != NULL){
        	for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){

                	if(gn[i] == NULL)
                        	continue;

                	for(j=0; j< gn[i]->results.numberOfCommunities; j++)
                        	free(gn[i]->results.communities[j].members);
                	free(gn[i]->results.communities);
                	free(gn[i]->forum);
        	}
        	free(gn);
	}

	if(cpm != NULL){
        	for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){

                	if(cpm[i] == NULL)
                        	continue;

                	for(j=0; j< cpm[i]->clique3.numberOfCommunities; j++)
                        	free(cpm[i]->clique3.communities[j].members);
                	free(cpm[i]->clique3.communities);

                	for(j=0; j< cpm[i]->clique4.numberOfCommunities; j++)
                        	free(cpm[i]->clique4.communities[j].members);
                	free(cpm[i]->clique4.communities);
                	free(cpm[i]->forum);
        	}
        	free(cpm);
	}
}

boolean validateTopNResults(forum_data **forums) {
	int i;
	forum_data **actualResults = topN();

	if(forums == NULL){
		printf("Top N Forums results is empty!!!\n");
		return False;
	}

	forum_data *results = malloc(NUMBER_OF_TOP_N_FORUMS*sizeof(forum_data));
	for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i)
		results[i] = *forums[i];

	qsort(results, NUMBER_OF_TOP_N_FORUMS, sizeof(forum_data), compareForums);

	for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){
		if(results[i].size != actualResults[i]->size || (strcmp(results[i].name, actualResults[i]->name) != 0)){
			i++;
			printf("The top %d forum has size:%d and name:\"%s\" and not size:%d and name:\"%s\"\n", i,
				actualResults[i-1]->size, actualResults[i-1]->name, results[i-1].size, results[i-1].name);
			return False;
		}
	}
	free(results);
	return True;
}

boolean validateCPMResults(Cpm **cpm){

	FILE *file = fopen("Cpm3Results.txt", "w");

	if(cpm == NULL)
		return False;

	int i, j, k;
	for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){

		for(j=0; j< cpm[i]->clique3.numberOfCommunities; ++j){

			if(cpm[i]->clique3.communities[j].numberOfMembers > 0){

				fprintf(file, "Community in forum %s:", cpm[i]->forum);

				qsort(cpm[i]->clique3.communities[j].members,
						cpm[i]->clique3.communities[j].numberOfMembers,
						sizeof(int), compareIntegers);

				for(k=0; k< cpm[i]->clique3.communities[j].numberOfMembers; ++k)
					fprintf(file," %d", cpm[i]->clique3.communities[j].members[k]);
				fprintf(file,"\n");
			}
		}
	}
	fclose(file);

	char script[61];
        strcpy(script, "sort Cpm3Results.txt -o Cpm3Results.txt");
        system(script);
        strcpy(script, "diff Cpm3ActualResults.txt Cpm3Results.txt > results3CPM.txt");
        system(script);

	file = fopen("results3CPM.txt", "r");
	fseek(file, 0, SEEK_END);
        if(ftell(file) != 0){
		printf("Results for CPM for size 3 are not correct\n");
		return False;
	}
	fclose(file);

	file = fopen("Cpm4Results.txt", "w");
        for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){

                for(j=0; j< cpm[i]->clique4.numberOfCommunities; ++j){

                        if(cpm[i]->clique4.communities[j].numberOfMembers > 0){

                                fprintf(file, "Community in forum %s:", cpm[i]->forum);

                                qsort(cpm[i]->clique4.communities[j].members,
                                                cpm[i]->clique4.communities[j].numberOfMembers,
                                                sizeof(int), compareIntegers);

                                for(k=0; k< cpm[i]->clique4.communities[j].numberOfMembers; ++k)
                                        fprintf(file," %d", cpm[i]->clique4.communities[j].members[k]);
                                fprintf(file,"\n");
                        }
                }
        }
        fclose(file);

	memset(script, '\0', 61*sizeof(char));
        strcpy(script, "sort Cpm4Results.txt -o Cpm4Results.txt");
        system(script);
        strcpy(script, "diff Cpm4ActualResults.txt Cpm4Results.txt > results4CPM.txt");
        system(script);

        file = fopen("results4CPM.txt", "r");
        fseek(file, 0, SEEK_END);
        if(ftell(file) != 0){
                printf("Results for CPM for size 4 are not correct\n");
		return False;
        }
	fclose(file);

	return True;
}

boolean validateGNResults(Gn **gn){

	 FILE *file = fopen("GNResults.txt", "w");

        if(gn == NULL)
                return False;

        int i, j, k;
        for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){

                for(j=0; j< gn[i]->results.numberOfCommunities; ++j){

                        if(gn[i]->results.communities[j].numberOfMembers > 0){

                                fprintf(file, "Community in forum %s:", gn[i]->forum);

                                qsort(gn[i]->results.communities[j].members,
                                                gn[i]->results.communities[j].numberOfMembers,
                                                sizeof(int), compareIntegers);

                                for(k=0; k< gn[i]->results.communities[j].numberOfMembers; ++k)
                                        fprintf(file," %d", gn[i]->results.communities[j].members[k]);
                                fprintf(file,"\n");
                        }
                }
        }
	fclose(file);

	char script[55];
        strcpy(script, "sort GNResults.txt -o GNResults.txt");
        system(script);
        strcpy(script, "diff GNActualResults.txt GNResults.txt > resultsGN.txt");
        system(script);

        file = fopen("resultsGN.txt", "r");
        fseek(file, 0, SEEK_END);
        if(ftell(file) != 0){
                printf("Results for GN are not correct\n");
		return False;
        }
	fclose(file);

	return True;
}



forum_data** getTopNForums(){

	int i;
	forum_data **forums;
	forums = malloc(NUMBER_OF_TOP_N_FORUMS*sizeof(forum_data));

	for(i=0; i<NUMBER_OF_TOP_N_FORUMS; ++i){

			if(i == 0){
					forums[i].size = 82;
		forums[i]->name = malloc(26*sizeof(char));
					strcpy(forums[i].name, "Wall of Xiomara Fernandez");
			} else if(i == 1){
					forums[i]->size = 72;
		forums[i]->name = malloc(30*sizeof(char));
					strcpy(forums[i].name, "Album 13 of Xiomara Fernandez");
			}else if(i == 2){
					forums[i]->size = 62;
		forums[i]->name = malloc(29*sizeof(char));
					strcpy(forums[i].name, "Album 4 of Xiomara Fernandez");
			}else if(i == 3){
					forums[i]->size = 17;
		forums[i]->name = malloc(17*sizeof(char));
					strcpy(forums[i].name, "Wall of R. Singh");
			}else if(i == 5){
					forums[i]->size = 15;
		forums[i]->name = malloc(22*sizeof(char));
					strcpy(forums[i].name, "Best Of Gordon Murray");
			}else {
					forums[i]->size = 15;
		forums[i]->name = malloc(30*sizeof(char));
					strcpy(forums[i].name, "Album 18 of Xiomara Fernandez");
			}
	}
	return forums;
	}
*/