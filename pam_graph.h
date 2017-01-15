//pam_graph.h
//Anapty3h logismikou - Ergasia 1

#ifndef PAM_GRAPH_H_
#define PAM_GRAPH_H_

#include "data_storage_utilities.h"

//1. YLOPOIHMENES DOMES

typedef enum {false=0, true} bool;

//node_type
typedef enum {	
	person=0,
	comment,
	forum,
	tag,
	tagclass,
	organisation,
	place,
	post,
	
	language,
	email,
	hypernode,	//Xrhsimopoieitai apo to 3 meros ths ergasias
	invalid_node = -1

#define NUM_OF_TYPES (hypernode - person + 1)

	} node_type;	//hypernode panta teleytaio kai person panta prwto! (Gia na metrame to mege8os!)

//relationship_type
typedef enum {
	//Na MHN allaxtei h seira! (logw load_relationship_csv())
	//id
	post_hasCreator_person = 0,
	forum_containerOf_post,
	comment_replyOf_post,
	comment_hasCreator_person,
	person_knows_person,
	person_isLocatedIn_place,
	person_hasInterest_tag,
	//id + data
	forum_hasMember_person,
	person_workAt_organisation,
	person_studyAt_organisation,
	person_likes_post,
	//data
	person_speaks_language,
	person_email_emailaddress,
	
	//Extra
	hyperedge,
	trusts,
	invalid_relationship = -1
	} relationship_type;

//pam_cb_result
typedef struct pam_cb_result{
	int id;
	double cb;
	} pam_cb_result;

//pam_properties
typedef struct pam_properties{
	int num_of_properties;
	void** my_properties;
	} pam_properties;

//pam_relationship
typedef struct pam_relationship{
	//H next_relationship PREPEI na arxikopoieitai se NULL
	relationship_type my_relationship_type;
	pam_properties* my_properties;
	struct pam_relationship* next_relationship;
	} pam_relationship;

//pam_lite_edge
typedef struct pam_lite_edge{
	//Auth h domh apo8hkevei thn plhroforia ths akmhs
    //Den apo8hkeuoume arxiko komvo, afou h insertEdge exei san orisma ton arxiko komvo
    int ending_id;  //O komvos pou 8a katalhgei h akmh
    pam_relationship* their_relationship;
    struct pam_lite_edge* next_pam_edge;
	} pam_lite_edge;

//pam_edge
typedef struct pam_edge{
	//Ayth h domh leitourgei ws wrapper gia akmes (pam_lite_edge)
	node_type my_node_type;
	int starting_id;
	pam_lite_edge* my_edge;
	} pam_edge;

//pam_node
typedef struct{
	int id;
	int type;
	pam_properties* my_properties;
	} pam_node;

//entity_list
typedef struct entity_list{
	node_type my_type;
	int num_of_edges;
	pam_lite_edge* my_edges;
	} entity_list;

//pam_cell
typedef struct{
    pam_node* my_node;
	entity_list* my_list;
	} pam_cell;

//pam_bucket
typedef struct{
    int current_size;   //to trexon mege8os, pou metabaletai se periptwsh overflow
    int occupied_cells;
    pam_cell* my_cells;
	} pam_bucket;

//pam_graph
typedef struct{
	node_type my_node_type;
    int initial_size;   //Arxiko mege8os hash table (m)
    int current_size;   //Trexon mege8os hash table
    int num_of_cells;   //Kelia ana kouva (c)
    pam_bucket** hashtable;    //Deikths sto hash table
    int current_level;  //Trexon epipedo dhmioyrgias split (i)
    int split_bckt;		// next bucket to split
	int num_of_nodes;	//Metrhths komvwn
	int num_of_edges;	//Metrhths akmwn
	int* num_of_edges_type;
	pam_cb_result* betweeness_centrality_results;
	} pam_graph;

//node_entry
typedef struct node_entry{
	//Xrhsimopoieitai apo to betweenessCentrality
	pam_cell* my_element;
	double cb;
	double delta;
	int sigma;
	int d;
	pam_list* P;
	} node_entry;

//cell_map
typedef struct cell_map{
	//Xrhsimopoieitai gia na mporoume na prospelaunoume tous komvous enos grafou
	int bucket_num;
	int cell_num;
	}cell_map;
	
//fringe
typedef struct fringe{
	pam_cell** the_fringe;
	pam_cell** temp_fringe; //Xrhsimopoieitai gia na apo8hkeyoume prosorina ta kainouria synora apo ton arxiko komvo
	double *the_weight; // gia grafous me barh mono. megethous fringe size/counter
	double *temp_weight;
	int fringe_size;
	int the_fringe_counter;
	int temp_fringe_counter;
	int* visited;	//Edw apo8hkeyontai oi komvoi poy exoyn episkeytei apo thn arxh
	int visited_size;
	int visited_counter;
	} fringe;

//pam_resultSet
typedef struct{
	fringe *my_fringe;
	int *constraints;
	char mode;
	int current_node; // o epomenos komvos pou 8a vgei apo to sunoro
	int dist; // h trexousa apostash apo ton arxiko komvo
	pam_graph* my_graph;
	} pam_resultSet;

//pam_result
typedef struct{
    int id;
    int distance;
	} pam_result;

//pam_double_result
typedef struct{
	pam_result p_m;
	double weight;
	} pam_D_result;
	


//2. YLOPOIHMENES SYNARTHSEIS


//Utilities: 
void pam_print_graph(pam_graph* my_graph);

int empty_graph(pam_graph *g);

void set_constraints(pam_resultSet *resSet, int *constraints, char mode);

pam_lite_edge *pam_free_edge(pam_lite_edge *current_edge);

pam_properties* pam_createProperties(int number);

void pam_setStringProperty(char* property, int index, pam_properties* p);

void pam_setIntegerProperty(int property, int index, pam_properties* p);

void pam_setDoubleProperty(double property, int index, pam_properties* p);

pam_node* pam_createNode(int id, pam_properties* p);

pam_edge* pam_createEdge(int startID, int endID, pam_properties* p, node_type my_node_type, relationship_type my_relationship_type);

cell_map* create_cell_map();

void destroy_cell_map(cell_map* my_cell_map);

pam_cell* get_next_cell(cell_map* my_cell_map, pam_graph* my_graph);

void reset_cell_map(cell_map* my_cell_map);



//Utilities: Node List
pam_list* create_node_list(pam_graph *graph);
	
void node_list_reset(pam_list* my_node_list);
	
void destroy_node_list(pam_list* my_node_list);

node_entry* get_node_with_id(int id, pam_list* my_node_list);

node_entry* get_node_with_offset(int offset, pam_list* my_node_list);


//Zhtoumenes
pam_graph* pam_createGraph(int initial_hashtable_size, int num_of_cells, node_type my_node_type);

bool pam_destroyliteGraph(pam_graph* my_graph, char mode);

bool pam_destroyGraph(pam_graph* my_graph);

bool pam_insertNode(pam_graph* my_graph, pam_node* my_node);

bool pam_insertEdge(pam_graph* my_graph, pam_edge* my_edge);

pam_cell* pam_offsets(pam_graph* my_graph, int my_id, int *i, int *j);

pam_cell* pam_lookup_cell(pam_graph* my_graph, int my_id);

pam_node* pam_lookupNode(pam_graph* my_graph, int my_id);

double pam_reachNode2(pam_graph* my_graph, int my_starting_id, int my_ending_id, char mode);

int pam_reachNode1(pam_graph* my_graph, int my_starting_id, int my_ending_id);

pam_resultSet* pam_reachNodesN(pam_graph* my_graph, int my_id);

bool pam_next(pam_resultSet* my_resultSet, pam_result* pair);

void pam_destroy_resultSet(pam_resultSet* my_resultSet);




#endif
