/*
 * defines.h
 *
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include "limits.h"

#define MAX_STRING_LENGTH 256	/*maximum string length*/
#define INTEGER_SIZE 4			/*integer size*/

#define SUCCESS 0
#define INFINITY_REACH_NODE INT_MAX 

//Our constants
#define INITIAL_FRINGE_SIZE 10
#define DEFAULT_HASHTABLE_SIZE 4
#define DEFAULT_NUM_OF_CELLS 4
#define DECIMAL_DIGITS 1000000
#define NUMBER_OF_THREADS 4

//ReachNodes modes
#define DEFAULT 0 //normal reachnodesΝ requested at level 1
#define TL_TRUST 1 // for tidal trust algorithm
#define GN_BETWEEN 2 // girvan-newman: finds all paths in edge centrality
#define GN_EDGES 3 // girvan-newman: reach all edges

#endif /* DEFINES_H_ */
