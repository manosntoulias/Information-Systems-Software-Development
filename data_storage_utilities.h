//data_storage_utilities.h

#ifndef DATA_UTILITIES
#define DATA_UTILITIES

//UTILITIES

//1. DOMES DEDOMENWS
//Utilities: Stack Implementation
//pam_stack_element
typedef struct stack_element{
	void* my_value;
	struct stack_element* next_element;
	}pam_stack_element;

//pam_stack
typedef struct{
	pam_stack_element* top;
	int size;
	}pam_stack;

pam_stack* pam_create_stack();
void pam_destroy_stack(pam_stack* my_stack);
void pam_push_to_stack(pam_stack* my_stack, void* to_be_added);
void* pam_pop_from_stack(pam_stack* my_stack);
int get_stack_size(pam_stack* my_stack);



//Utilities: Queue Implementation
//pam_queue_element
typedef struct queue_element{
	void* my_value;
	struct queue_element* previous_element;
	}pam_queue_element;

//pam_queue
typedef struct{
	pam_queue_element* first;
	pam_queue_element* last;
	int size;
	}pam_queue;


pam_queue* pam_create_queue();
void pam_destroy_queue(pam_queue* my_queue);
void pam_push_to_queue(pam_queue* my_queue, void* to_be_added);
void* pam_pop_from_queue(pam_queue* my_queue);
int get_queue_size(pam_queue* my_queue);


//Utilities: List Implementation
//pam_list_element
typedef struct list_element{
	void* my_value;
	struct list_element* next_element;
	}pam_list_element;

//pam_list
typedef struct{
	pam_list_element* top;
	int size;
	}pam_list;

pam_list* pam_create_list();
void pam_destroy_list(pam_list* my_list);
void pam_push_to_list(pam_list* my_list, void* to_be_added);
void pam_append_to_list(pam_list* my_list, void* to_be_added);
void pam_push_to_list_offset(pam_list* my_list, void* to_be_added, int offset);
void* pam_get_from_list(pam_list* my_list, int offset);
void pam_remove_from_list(pam_list* my_list, int offset);
int get_list_size(pam_list* my_list);

#endif