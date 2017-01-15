//data_storage_utilities.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_storage_utilities.h"


//1. DOMES DEDOMENWN
pam_stack* pam_create_stack(){
	pam_stack* my_stack;
	
	if((my_stack = malloc(sizeof(pam_stack)))==NULL){
		printf("pam_createStack: Error allocating memory\n");
		return NULL;
		}
	
	my_stack->top = NULL;
	my_stack->size = 0;
	
	return my_stack;
	}
	
void pam_destroy_stack(pam_stack* my_stack){
	while( pam_pop_from_stack(my_stack) != NULL){}
	free(my_stack);
	}

void pam_push_to_stack(pam_stack* my_stack, void* to_be_added){
	pam_stack_element* my_element;
	
	if((my_element = malloc(sizeof(pam_stack_element)))==NULL){
		printf("pam_push: Error allocating memory\n");
		return;
		}
	
	my_element->my_value = to_be_added;
	my_element->next_element = my_stack->top;
	
	my_stack->top = my_element;
	my_stack->size++;
	}
	
void* pam_pop_from_stack(pam_stack* my_stack){
	pam_stack_element* my_element;
	void* my_value;
	
	if(my_stack->size==0){
		return NULL;
		}
	else{
		my_element = my_stack->top;
		my_stack->top = my_element->next_element;
		my_value = my_element->my_value;
		free(my_element);
		my_stack->size--;
		
		}
	
	return my_value;
	}

int get_stack_size(pam_stack* my_stack){
	if(my_stack != NULL){
		return my_stack->size;
		}
	else{
		return -1;
		}
	}
	


//Utilities: Queue Implementation
pam_queue* pam_create_queue(){
	pam_queue* my_queue;
	
	if((my_queue = malloc(sizeof(pam_queue)))==NULL){
		printf("pam_queue: Error allocating memory\n");
		return NULL;
		}
	
	my_queue->first = NULL;
	my_queue->last = NULL;
	my_queue->size = 0;
	
	return my_queue;
	}
	
void pam_destroy_queue(pam_queue* my_queue){	
	while(pam_pop_from_queue(my_queue) != NULL){}	
	free(my_queue);
	}
	
void pam_push_to_queue(pam_queue* my_queue, void* to_be_added){
	pam_queue_element* my_element;
	
	if((my_element = malloc(sizeof(pam_queue_element)))==NULL){
		printf("pam_push_to_queue: Error allocating memory\n");
		return;
		}
		
	my_element->my_value = to_be_added;
	my_element->previous_element = NULL;
	
	if(my_queue->size == 0){
		my_queue->first = my_element;
		my_queue->last = my_element;
		}
	else{
		my_queue->last->previous_element = my_element;
		my_queue->last = my_element;
		}
	
	my_queue->size++;
	}
	
void* pam_pop_from_queue(pam_queue* my_queue){
	pam_queue_element* my_element;
	void* my_value;
	
	if(my_queue->size==0){
		return NULL;
		}
	else{
		my_element = my_queue->first;
		my_queue->first = my_element->previous_element;
		
		if(my_queue->size==1){
			my_queue->last = my_queue->first;
			}
		
		my_value = my_element->my_value;
		free(my_element);
		my_queue->size--;
		
		return my_value;
		}
	}

int get_queue_size(pam_queue* my_queue){
	if(my_queue != NULL){
		return my_queue->size;
		}
	else{
		return -1;
		}
	}
	
	
	
//Utilities: List Implementation
pam_list* pam_create_list(){
	pam_list* my_list;
	
	if((my_list = malloc(sizeof(pam_list)))==NULL){
		printf("pam_list: Error allocating memory\n");
		return NULL;
		}
	
	my_list->top = NULL;
	my_list->size = 0;
	
	return my_list;
	}
void pam_destroy_list(pam_list* my_list){
	pam_list_element* current_element, *next_element;
	
	current_element = my_list->top;
	while(current_element != NULL){
		next_element = current_element->next_element;
		free(current_element);
		current_element = next_element;
		}
	free(my_list);
	}
void pam_push_to_list(pam_list* my_list, void* to_be_added){
	pam_list_element* my_element;
	
	if((my_element = malloc(sizeof(pam_list_element)))==NULL){
		printf("pam_push_to_list: Error allocating memory\n");
		return;
		}
		
	my_element->my_value = to_be_added;
	my_element->next_element = my_list->top;
	
	my_list->top = my_element;
	my_list->size++;
	}
void pam_push_to_list_offset(pam_list* my_list, void* to_be_added, int offset){
	pam_list_element* my_element, * current_element, * next_element;
	int i;
	
	if(offset>my_list->size){
		printf("pam_push_to_list_offset: Invalid input\n");
		printf("pam_push_to_list_offset: List size: %d\tOffset: %d\n", my_list->size, offset);
		return;
		}
	
	if((my_element = malloc(sizeof(pam_list_element)))==NULL){
		printf("pam_push_to_list: Error allocating memory\n");
		return;
		}
	
	my_element->my_value = to_be_added;
	
	if(offset == 0){
		next_element = my_list->top;
		my_element->next_element = next_element;
		my_list->top = my_element;
		my_list->size++;
		return;
		}
	
	current_element = my_list->top;
	for(i=1; i<offset; i++){	//3ekiname apo to 1 dioti to 0 eleg8hke parapanw
		//printf("i = %d/%d\tcurrent_element: %d\n", i, offset, *(int*)current_element->my_value);
		current_element = current_element->next_element;
		}
	  my_element->next_element = current_element->next_element;
	  current_element->next_element = my_element;
	  my_list->size++;
	  
	  return;
	  }
void pam_append_to_list(pam_list* my_list, void* to_be_added){
	pam_push_to_list_offset(my_list, to_be_added, my_list->size);
	}
void* pam_get_from_list(pam_list* my_list, int offset){
	pam_list_element* my_element;
	int i;
	
	if(offset >= my_list->size){
		printf("pam_get_from_list: Invalid offset\n");
		return NULL;
		}
	else{
		my_element = my_list->top;
		for(i=0; i<offset; i++){
			my_element = my_element->next_element;
			}
		return my_element->my_value;
		}
	}

void pam_remove_from_list(pam_list* my_list, int offset){
	pam_list_element* current_element, * next_element;
	int i;
	
	if(offset>my_list->size){
		printf("pam_remove_from_list: Invalid input\n");
		printf("pam_remove_from_list: List size: %d\tOffset: %d\n", my_list->size, offset);
		return;
		}
	
	my_list->size--;
		
	if(offset == 0){
		current_element = my_list->top;
		next_element = current_element->next_element;
		my_list->top = next_element;
		free(current_element);
		return;
		}
	
	current_element = my_list->top;
	for(i=1; i<offset; i++){	//3ekiname apo to 1 dioti to 0 eleg8hke parapanw
		//printf("i = %d/%d\tcurrent_element: %d\n", i, offset, *(int*)current_element->my_value);
		current_element = current_element->next_element;
		}
	next_element = current_element->next_element;
	if(next_element != NULL){
		current_element->next_element = next_element->next_element;
		}
	else{
		if(my_list->size == 0){
			my_list->top = NULL;
			}
		current_element->next_element = NULL;
		}
	free(current_element);
	return;
	}
	
int get_list_size(pam_list* my_list){
	if(my_list != NULL){
		return my_list->size;
		}
	else{
		return -1;
		}
	}

	