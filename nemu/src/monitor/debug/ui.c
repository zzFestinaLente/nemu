#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_help(char *args);
static int cmd_x(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Run the cpu_exec function for a specified number of steps", cmd_si},
	{ "info", "Display the current values of all registers or watchpoints", cmd_info},
	{ "x", "Print address content", cmd_x},

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args) {
    int step = 1;  // 默认步数为1
    if (args != NULL) {
        sscanf(args, "%d", &step);  // 如果提供了参数则解析
    }
    cpu_exec(step);  // 执行指定步数
    return 0;
}

static int cmd_info(char *args) {
    // 如果有参数传入且第一个字符为 'r'，打印寄存器的值
	int i;
    if (args != NULL && args[0] == 'r') {
        // 遍历并打印 8 个通用寄存器的名称、16 进制和 10 进制值
        for (i = 0; i < 8; i++) {
            printf("%s\t  0x%08x\t  %d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
        }
        // 打印 eip（指令指针）的值，同样是 16 进制和 10 进制
        printf("eip\t  0x%08x\t  %d\n", cpu.eip, cpu.eip);
    } 
    return 0;
}

static int cmd_x(char *args) {

	int len;
	int value;
	int base_addr;
    char *len_str = strtok(args, " ");
    char *expr_str = strtok(NULL, "");
	int i;
    // 将长度字符串转化为整数
    sscanf(len_str, "%d", &len);
    // 解析表达式获取内存起始地址
    base_addr = expr(expr_str, 0);
    // 循环读取并输出内存数据
    for (i = 0; i < len; i++) {
        // 每4个字输出一个内存地址
        if (i % 4 == 0) {
            printf("0x%x: ", base_addr + 4 * i);
        }
        // 读取并打印每个地址的数据
        value = swaddr_read(base_addr + 4 * i, sizeof(int));
        printf("0x%08x  ", value);
        // 每4个字换行，或在最后一个元素时换行
        if ((i + 1) % 4 == 0 || i == len - 1) {
            printf("\n");
        }
    }

    return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
