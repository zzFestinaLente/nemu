#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, 
	//added token type
	EQ,AND,OR,
	ADDRESS,DEREF,NEGT,
	NEQ,REGISTER,HEXNUM,DECNUM
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},                // 匹配空格，忽略
	{"\\+", '+'},                   // 加法运算符 +
	{"\\-", '-'},                   // 减法运算符 -
	{"\\*", '*'},                   // 乘法运算符 *
	{"\\/", '/'},                   // 除法运算符 /
	{"\\(", '('},                   // 左括号 (
	{"\\)", ')'},                   // 右括号 )
	{"==", EQ},                     // 相等 ==
	{"!=", NEQ},                    // 不等于 !=
	{"&&", AND},                    // 逻辑与 &&
	{"\\|\\|", OR},                 // 逻辑或 ||
	{"\\!", '!' },                  // 逻辑非 !
	{"\\$[a-zA-Z]+", REGISTER},     // 寄存器名，如 $eax, $ebx
	{"0x[0-9A-Fa-f]+", HEXNUM},     // 16进制数字，如 0x1A3F
	{"[0-9]+", DECNUM},             // 10进制数字，如 123
	{"\\*0x[0-9A-Fa-f]+", ADDRESS}, // 地址解引用，*0x1234
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

// 用于判断前一个token是否为数字或寄存器的辅助函数
static bool is_prev_token_num_or_reg(int m) {
    return (tokens[m - 1].type == DECNUM || tokens[m - 1].type == HEXNUM || tokens[m - 1].type == REGISTER || tokens[m - 1].type == ')');
}

static bool make_token(char *e) {
    int position = 0;
    regmatch_t pmatch;
    
    nr_token = 0;

    while (e[position] != '\0') {
        bool matched = false;

        // 尝试所有正则表达式规则
		int i;
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                int substr_len = pmatch.rm_eo;
                char *substr_start = e + position;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

                // 记录匹配到的token并进行必要的字符串处理
                if (rules[i].token_type != NOTYPE) {
                    Token *token = &tokens[nr_token++];
                    token->type = rules[i].token_type;
                    snprintf(token->str, sizeof(token->str), "%.*s", substr_len, substr_start);
                }

                position += substr_len;
                matched = true;
                break;
            }
        }

        // 如果没有规则匹配，返回false
        if (!matched) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    // 遍历token数组，处理特殊情况：解引用(*)和负号(-)
	int m;
    for (m = 0; m < nr_token; m++) {
        Token *token = &tokens[m];
        if (token->type == '*' && (m == 0 || !is_prev_token_num_or_reg(m))) {
            token->type = DEREF;
        } else if (token->type == '-' && (m == 0 || !is_prev_token_num_or_reg(m))) {
            token->type = NEGT;
        }
    }

    return true;
}

bool check_parentheses(int p, int q) {
    int leftp = 0;

    // 首先判断开头和结尾的括号是否匹配
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }
    // 遍历从 p+1 到 q-1 的部分
	int i;
    for (i = p + 1; i < q; i++) {
        if (tokens[i].type == '(') {
            leftp++;
        } else if (tokens[i].type == ')') {
            leftp--;
            // 如果左括号数量少于右括号，说明不匹配
            if (leftp < 0) {
                return false;
            }
        }
    }
    // 检查左括号和右括号的数量是否相等
    return leftp == 0;
}


int get_priority(int type);
int handle_unary_op(int type, int start, int end);
int calculate_result(int type, int val1, int val2);

int eval(int p, int q) {
    int result;
    
    if (p > q) {
        assert(0);
		return 0;
    } 
	if (p == q) {
        switch (tokens[p].type) {
            case REGISTER:
                if (strcmp(tokens[p].str + 1, "eip") == 0) {
					return cpu.eip;
				}
                int k;
                for (k = 0; k < 8; k++) {
                    if (strcmp(tokens[p].str + 1, regsl[k]) == 0)
                        return cpu.gpr[k]._32;
                }
                break;
            case HEXNUM:
                sscanf(tokens[p].str, "%x", &result);
                return result;
            case DECNUM:
                sscanf(tokens[p].str, "%d", &result);
                return result;
            case ADDRESS:
                sscanf(tokens[p].str, "%x", &result);
                return swaddr_read(result, 4);
        }
		
    } else if (check_parentheses(p, q)) {
        return eval(p + 1, q - 1);
    } else {
        int op = 0, left = 0, cha0 = 0, j;
        for (j = p; j <= q; j++) {
            if (tokens[j].type == '(') {
                left++;
            } else if (tokens[j].type == ')') {
                left--;
            } else if (left == 0 && tokens[j].type != DECNUM && tokens[j].type != HEXNUM && tokens[j].type != ADDRESS && tokens[j].type != REGISTER) {
                int current_priority = get_priority(tokens[j].type);
                if (cha0 == 0) {
                    op = j;
                    cha0 = 1;
                } else if (get_priority(tokens[op].type) <= current_priority) {
                    op = j;
                }
            }
        }

        if (tokens[op].type == '!' || tokens[op].type == DEREF || tokens[op].type == NEGT) {
            return handle_unary_op(tokens[op].type, op + 1, q);
        }

        int val1 = eval(p, op - 1);
        int val2 = eval(op + 1, q);

        return calculate_result(tokens[op].type, val1, val2);
    }
    return 0;
}

int get_priority(int type) {
    switch (type) {
        case '(': case ')': return 1;
        case '!': case DEREF: case NEGT: return 2;
        case '*': case '/': return 3;
        case '+': case '-': return 4;
        case EQ: case NEQ: return 5;
        case AND: return 6;
        case OR: return 7;
        default: return 0;
    }
}

int handle_unary_op(int type, int start, int end) {
    switch (type) {
        case '!': return !eval(start, end);
        case DEREF: return swaddr_read(eval(start, end), 4);
        case NEGT: return -eval(start, end);
        default: assert(0); return 0;
    }
}

int calculate_result(int type, int val1, int val2) {
    switch (type) {
		case EQ: return val1 == val2;
        case NEQ: return val1 != val2;
        case AND: return val1 && val2;
        case OR: return val1 || val2;
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        default: assert(0); 
		return 0;
    }
}


uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0, nr_token - 1);
	// panic("please implement me");
	// return 0;
}

