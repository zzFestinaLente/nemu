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

	{" +",  NOTYPE},              // 匹配一个或多个空格（忽略）
    {"\\+", '+'},                 // 匹配 '+' 运算符
    {"==", EQ},                   // 匹配等于运算符 '=='

    {"[0-9]+", DECNUM},          // 匹配十进制数字（一个或多个数字）
    {"\\*0x[0-9A-Fa-f]+", ADDRESS}, // 匹配十六进制地址（例如 *0x1A3F）
    {"&&", AND},                  // 匹配逻辑与运算符 '&&'
    {"\\|\\|", OR},               // 匹配逻辑或运算符 '||'
    {"!=", NEQ},                  // 匹配不等于运算符 '!='
    {"\\!", '!'},                 // 匹配逻辑非运算符 '!'
    {"\\$[a-zA-Z]+", REGISTER},   // 匹配寄存器名称（例如 $eax）
    {"0x[0-9A-Fa-f]+", HEXNUM},  // 匹配十六进制数字（例如 0x1A3F）
    {"\\-", '-'},                 // 匹配 '-' 运算符
    {"\\*", '*'},                 // 匹配 '*' 运算符（乘法）
    {"\\/", '/'},                 // 匹配 '/' 运算符（除法）
    {"\\(", '('},                 // 匹配左括号 '('
    {"\\)", ')'},                 // 匹配右括号 ')'
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

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				// switch(rules[i].token_type) {
				// 	default: panic("please implement me");
				// }
				switch(rules[i].token_type) {
					case NOTYPE:
					     break;
					default: 
					     tokens[nr_token].type = rules[i].token_type;
					     sprintf (tokens[nr_token].str, "%.*s", substr_len, substr_start);
					     nr_token++;
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
		int m;
		for (m = 0; m < nr_token; m++) {
		if (tokens[m].type == '*' &&( m == 0 || (tokens[m - 1].type != DECNUM && tokens[m - 1].type != HEXNUM && tokens[m - 1].type != REGISTER && tokens[m - 1].type != ')')))
			tokens[m].type = DEREF;
		if (tokens[m].type == '-' &&( m == 0 || (tokens[m - 1].type != DECNUM && tokens[m - 1].type != HEXNUM && tokens[m - 1].type != REGISTER && tokens[m - 1].type != ')')))
			tokens[m].type = NEGT;
		}
	}
	
	return true; 
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

