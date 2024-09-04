#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;  //head 用于组织使用中的监视点结构,free_用于组织空闲的监视点结构

void init_wp_pool() {  //init_wp_pool()函数会对head free两个链表进行初始化
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
#define MAX_CONT_SIZE 256  // 根据需要调整大小

// WP* new_wp(char *cont) {
// 	if (free_ == NULL)  assert(0);
// 	WP *roy = free_;
// 	free_ = free_ -> next;
// 	roy -> next = NULL;
// 	bool success = true;
// 	strcpy(roy -> cont, cont);  //表达式
// 	roy -> val = expr(roy -> cont, &success); //表达式的值
// 	if (head == NULL){
// 		head = roy;
// 	}
// 	else if (head != NULL) {
// 		WP *p = head;
// 		while (p -> next != NULL) {
// 			p = p -> next;
// 		}
// 		p -> next = roy;
// 	}
// 	return roy;
// }
WP* new_wp(char* cont) {
    if (free_ == NULL) {
        fprintf(stderr, "No available watchpoints!\n");
        return NULL; // 如果没有可用的观察点，返回 NULL
    }

    WP* roy = free_;
    free_ = free_->next;
    roy->next = NULL;

    // 确保 cont 的长度不会超出 roy->cont 的缓冲区
    if (strlen(cont) >= MAX_CONT_SIZE) {
        fprintf(stderr, "Content too large!\n");
        free_wp(roy->NO);  // 释放分配的观察点
        return NULL;
    }

    strcpy(roy->cont, cont);  // 复制内容
    bool success = true;
    roy->val = expr(roy->cont, &success); // 计算表达式的值

    if (!success) {
        fprintf(stderr, "Expression evaluation failed!\n");
        free_wp(roy->NO);  // 释放分配的观察点
        return NULL;
    }

    // 将 roy 添加到链表末尾
    if (head == NULL) {
        head = roy;
    } else {
        WP* p = head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = roy;
    }

    return roy;
}
// int free_wp(int numb) {
// 	WP *p = head;
// 	WP *q = head -> next;
// 	if (head == NULL) {
// 		printf("No input!\n");
// 		return 0;
// 	}
// 	else if (numb == head -> NO) {
// 		head = head -> next;
// 	}
// 	else {
// 		while (numb != q -> NO && q -> next != NULL) {
// 			p = p -> next;
// 			q = q -> next;
// 		}
// 		if (q -> next == NULL) {
// 			printf("No such watchpoint!\n");
// 			return 0;
// 		}
// 		else p -> next = q -> next;	
// 	}
// 	WP *wp = q;
// 	wp -> next = free_;
// 	free_ = wp;
// 	return 1;
// }
int free_wp(int numb) {
    if (head == NULL) {
        fprintf(stderr, "No watchpoints to free!\n");
        return 0; // 如果链表为空，返回 0
    }

    WP* p = head;
    WP* q = NULL;

    if (head->NO == numb) {
        q = head;
        head = head->next;
    } else {
        while (p->next != NULL && p->next->NO != numb) {
            p = p->next;
        }
        if (p->next == NULL) {
            fprintf(stderr, "No such watchpoint!\n");
            return 0; // 找不到指定的观察点，返回 0
        }
        q = p->next;
        p->next = q->next;
    }

    // 将释放的观察点添加到 free_ 链表中
    q->next = free_;
    free_ = q;

    return 1;
}

/* TODO: Implement the functionality of watchpoint */
// int judge_wp() {
// 	WP *test = head;
// 	bool success = true;
// 	int resl = 0, j0 = 0;
// 	while (test != NULL) {
// 		resl = expr(test -> cont, &success);
// 		if (resl != test -> val) {
// 			printf("Hint watchpoint %d at address 0x%08x, expr = %s\n", test -> NO, expr("$eip", &success), test -> cont);
// 			printf("old value = 0x%08x\n", test -> val);
// 			printf("new value = 0x%08x\n", resl);
// 			test -> val = resl;
// 			j0 = -1;
// 		}
// 		test = test -> next;
// 	}
// 	return j0;
// }
int judge_wp() {
    if (head == NULL) {
        fprintf(stderr, "No watchpoints to check!\n");
        return 0; // 没有观察点，返回 0
    }

    WP* test = head;
    bool success = true;
    int resl = 0;
    int status = 0;

    while (test != NULL) {
        resl = expr(test->cont, &success);
        
        if (!success) {
            fprintf(stderr, "Failed to evaluate expression for watchpoint %d\n", test->NO);
            return -1; // 如果表达式计算失败，返回 -1
        }
        
        if (resl != test->val) {
            // 如果表达式的计算结果与观察点的值不一致，打印提示信息
            printf("Hint: watchpoint %d at address 0x%08x, expr = %s\n", test->NO, expr("$eip", &success), test->cont);
            test->val = resl; // 更新观察点的值
            status = -1; // 标记有观察点的值发生了变化
        }
        
        test = test->next; // 继续检查下一个观察点
    }
    
    return status;
}

