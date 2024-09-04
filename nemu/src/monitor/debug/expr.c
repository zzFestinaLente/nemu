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

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"\\-",'-'},                    //reduce
    {"\\*",'*'},                    //multiply
	{"\\/",'/'},                    //divide
	{"\\(",'('},                    //left(
	{"\\)",')'},                    //right)
	{"&&",AND},                    //logical and
	{"\\|\\|",OR},                  //logical or
	{"!=",NEQ},                     //not equal
	{"\\!",'!'},                    //logical not
	{"\\$[a-zA-Z]+",REGISTER},      //register
	{"0x[0-9A-Fa-f]+",HEXNUM},      //16nums
	{"[0-9]+",DECNUM},              //10nums
	{"\\*0x[0-9A-Fa-f]+",ADDRESS}  //address
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

bool check_parentheses(int p, int q) {
	int leftp = 0;
	int i;
	if (tokens[p].type != '(' || tokens[q].type != ')') {
		return false;
	}
	else {
		for (i = p; i <= q; i++) {
			if (leftp == 0 && i != q && i != p)  return false;
			if (tokens[i].type == '(')  leftp++;
			if (tokens[i].type == ')')  leftp--; 
		}
		if (leftp == 0)  return true;
		return false;
	}
}
int prior(int ty) {
	switch(ty) {
		case '(': return 1;
		case ')': return 1;
		case '!': return 2;
		case DEREF: return 2;
		case NEGT: return 2;
		case '*': return 3;
		case '/': return 3;
		case '+': return 4;
		case '-': return 4;
		case EQ: return 5;
		case NEQ: return 5;
		case AND: return 6;
		case OR: return 7;
	}
	return 0;
}
// int do_op(int p, int q) {
// 	int op0 = 0;
// 	int j, left = 0, cha0 = 0;
// 	for (j = p; j <= q; j++) {
// 		if (tokens[j].type == '(')  left++;
// 		else if (tokens[j].type == ')')  left--;
// 		else if (left == 0 && tokens[j].type != DECNUM && tokens[j].type != HEXNUM && tokens[j].type != ADDRESS && tokens[j].type != REGISTER) {
// 			if (cha0 == 0) {
// 				op0 = j;
// 				cha0 = 1;
// 			}
// 			else {
// 				if (prior(tokens[op0].type) <= prior(tokens[j].type)) {
// 					op0 = j;
// 				}
// 			}
// 		}
// 	}
// 	return op0;
// }
int eval (int p, int q) {
	int result;
	if (p > q) {
		assert(0);
	}
	else if (p == q) {
		if (tokens[p].type == REGISTER) {
			if (strcmp(tokens[p].str+1, "eip") == 0)  return cpu.eip;
			int k;
			for (k = 0; k < 8; k++) {
				if (strcmp(tokens[p].str+1, regsl[k]) == 0)
					return cpu.gpr[k]._32;
			}
		}
		if (tokens[p].type == HEXNUM) {
			sscanf(tokens[p].str, "%x", &result);
			return result;
		}
		if (tokens[p].type == DECNUM) {
			sscanf(tokens[p].str, "%d", &result);
			return result;
		}
		if (tokens[p].type == ADDRESS) {
			sscanf(tokens[p].str, "%x", &result);
			return swaddr_read(result, 4);
		}
	}
	else if (check_parentheses(p, q) == true) {
		return eval(p + 1, q - 1);
	}
	else {
		int op = 0;
		int j, left = 0, cha0 = 0;
		for (j = p; j <= q; j++) {
			if (tokens[j].type == '(')  left++;
			else if (tokens[j].type == ')')  left--;
			else if (left == 0 && tokens[j].type != DECNUM && tokens[j].type != HEXNUM && tokens[j].type != ADDRESS && tokens[j].type != REGISTER) {
				if (cha0 == 0) {
					op = j;
					cha0 = 1;
				}
				else {
					if (prior(tokens[op].type) <= prior(tokens[j].type)) {
						op = j;
					}
				}
			}
		}

		//printf("&&&&&&&&&&&&&&&&&&&%d\n", op);
		if (tokens[op].type == '!' || tokens[op].type == DEREF || tokens[op].type == NEGT) {
			if (tokens[op].type == '!')  return !eval(op + 1, q);
			if (tokens[op].type == DEREF)  return swaddr_read(eval(op + 1, q), 4);
			if (tokens[op].type == NEGT)  return -eval(op + 1, q);
		}
		int val1 = eval(p, op - 1);
		int val2 = eval(op + 1, q);
  		switch (tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case EQ: return val1 == val2;
			case AND: return val1 && val2;
			case OR: return val1 || val2;
			case NEQ: return val1 != val2;
			default: assert(0);
		}
	}
	return 0;
}



uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	// return evaluate(0, nr_token - 1);
	return eval(0, nr_token - 1);
	// panic("please implement me");
	// return 0;
}

