#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char cont[32];	
	uint32_t val;
	/* TODO: Add more members if necessary */


} WP;
WP* new_wp(char*);  
int free_wp(int);   
int judge_wp(); 
#endif
