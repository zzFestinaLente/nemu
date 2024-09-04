#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;  //NO 表示监视点序号
	struct watchpoint *next;  //尾指针
    char content[32];  //cont用来存储算数表达式的内容
	uint32_t value;   //用来存储算数表达式的结果
	/* TODO: Add more members if necessary */
} WP;

WP* new_wp(char*);  //从 free_链表中返回一个空闲的监视点结构
int free_wp(int);   //将 wp归还到 free_链表中
int judge_wp();  
void init_wp_pool();
int print_points();
void ui_mainloop();
#endif
