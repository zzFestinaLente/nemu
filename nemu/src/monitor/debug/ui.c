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
    char *arg1 = strtok(NULL, " ");  // 提取第一个参数（长度）
    char *arg2 = strtok(NULL, " ");  // 提取第二个参数（地址）

    int n;
    lnaddr_t addr;
    int i;

    if (arg1 != NULL && arg2 != NULL) {
        // 解析第一个参数（内存读取长度）
        sscanf(arg1, "%d", &n);

        // 解析第二个参数（地址），直接使用 sscanf
        sscanf(arg2, "%x", &addr);

        // 遍历读取内存
        for (i = 0; i < n; i++) {
            if (i % 4 == 0) {
                printf("0x%08x: ", addr);  // 输出地址
            }

            // 读取并输出地址处的 4 字节内容
            printf("0x%08x ", swaddr_read(addr, 4));
            addr += 4;

            if (i % 4 == 3) {
                printf("\n");  // 每 4 个输出换行
            }
        }

        // 如果最后没有正好换行，手动换行
        if (n % 4 != 0) {
            printf("\n");
        }
    } 
    else {
        printf("Invalid arguments\n");  // 参数不足
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
