#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}


WP* new_wp(char *cont) {
	if (free_ == NULL)  assert(0);
	WP *roy = free_;
	free_ = free_ -> next;
	roy -> next = NULL;
	bool success = true;
	strcpy(roy -> cont, cont);  //表达式
	roy -> val = expr(roy -> cont, &success); //表达式的值
	if (head == NULL){
		head = roy;
	}
	else if (head != NULL) {
		WP *p = head;
		while (p -> next != NULL) {
			p = p -> next;
		}
		p -> next = roy;
	}
	return roy;
}

int free_wp(int numb) {
	WP *p = head;
	WP *q = head -> next;
	if (head == NULL) {
		printf("No input!\n");
		return 0;
	}
	else if (numb == head -> NO) {
		head = head -> next;
	}
	else {
		while (numb != q -> NO && q -> next != NULL) {
			p = p -> next;
			q = q -> next;
		}
		if (q -> next == NULL) {
			printf("No such watchpoint!\n");
			return 0;
		}
		else p -> next = q -> next;	
	}
	WP *wp = q;
	wp -> next = free_;
	free_ = wp;
	return 1;
}
/* TODO: Implement the functionality of watchpoint */
int judge_wp() {
	WP *test = head;
	bool success = true;
	int resl = 0, j0 = 0;
	while (test != NULL) {
		resl = expr(test -> cont, &success);
		if (resl != test -> val) {
			printf("Hint watchpoint %d at address 0x%08x, expr = %s\n", test -> NO, expr("$eip", &success), test -> cont);
			printf("old value = 0x%08x\n", test -> val);
			printf("new value = 0x%08x\n", resl);
			test -> val = resl;
			j0 = -1;
		}
		test = test -> next;
	}
	return j0;
}
int print_points() {
	
	WP *Wang = head;
	while(Wang != NULL) {
		printf("%d\t  %s\t  %d\t\n", Wang -> NO, Wang -> cont, Wang -> val);
		Wang = Wang -> next;
	}
	return 0;
}



