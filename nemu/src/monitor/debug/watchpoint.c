#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *active_list, *free_list;  // active_list 用于组织使用中的监视点结构, free_list用于组织空闲的监视点结构

void init_wp_pool() {  // init_wp_pool()函数会对active_list和free_list两个链表进行初始化
    int i;
    for(i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    active_list = NULL;
    free_list = wp_pool;
}

#define MAX_CONT_SIZE 256  // 根据需要调整大小

WP* new_wp(char* content) {
    if (free_list == NULL) {
        fprintf(stderr, "No available watchpoints!\n");
        return NULL; // 如果没有可用的观察点，返回 NULL
    }

    WP* new_watchpoint = free_list;
    free_list = free_list->next;
    new_watchpoint->next = NULL;

    // 确保 content 的长度不会超出 new_watchpoint->content 的缓冲区
    if (strlen(content) >= MAX_CONT_SIZE) {
        fprintf(stderr, "Content too large!\n");
        free_wp(new_watchpoint->NO);  // 释放分配的观察点
        return NULL;
    }

    strcpy(new_watchpoint->content, content);  // 复制内容
    bool success = true;
    new_watchpoint->value = expr(new_watchpoint->content, &success); // 计算表达式的值

    if (!success) {
        fprintf(stderr, "Expression evaluation failed!\n");
        free_wp(new_watchpoint->NO);  // 释放分配的观察点
        return NULL;
    }

    // 将 new_watchpoint 添加到链表末尾
    if (active_list == NULL) {
        active_list = new_watchpoint;
    } else {
        WP* current = active_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_watchpoint;
    }

    return new_watchpoint;
}

int free_wp(int number) {
    if (active_list == NULL) {
        fprintf(stderr, "No watchpoints to free!\n");
        return 0; // 如果链表为空，返回 0
    }

    WP* current = active_list;
    WP* target = NULL;

    if (active_list->NO == number) {
        target = active_list;
        active_list = active_list->next;
    } else {
        while (current->next != NULL && current->next->NO != number) {
            current = current->next;
        }
        if (current->next == NULL) {
            fprintf(stderr, "No such watchpoint!\n");
            return 0; // 找不到指定的观察点，返回 0
        }
        target = current->next;
        current->next = target->next;
    }

    // 将释放的观察点添加到 free_list 链表中
    target->next = free_list;
    free_list = target;

    return 1;
}

int judge_wp() {
    if (active_list == NULL) {
        // fprintf(stderr, "No watchpoints to check!\n");
        return 0; // 没有观察点，返回 0
    }

    WP* current = active_list;
    bool success = true;
    int result = 0;
    int status = 0;

    while (current != NULL) {
        result = expr(current->content, &success);
        
        if (!success) {
            fprintf(stderr, "Failed to evaluate expression for watchpoint %d\n", current->NO);
            return -1; // 如果表达式计算失败，返回 -1
        }
        
        if (result != current->value) {
            // 如果表达式的计算结果与观察点的值不一致，打印提示信息
            printf("Hint watchpoint %d at address 0x%08x\n", current->NO, expr("$eip", &success));
            current->value = result; // 更新观察点的值
            status = -1; // 标记有观察点的值发生了变化
        }
        
        current = current->next; // 继续检查下一个观察点
    }
    
    return status;
}
