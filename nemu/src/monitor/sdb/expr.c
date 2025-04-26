/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "common.h"
#include <assert.h>
#include <isa.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,
    /* TODO: Add more token types */
    TK_NUM,
    TK_REG,
};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},   // spaces
    {"\\(", '('},        // parenthsis
    {"\\)", ')'},        // parenthsis
    {"\\+", '+'},        // plus
    {"-", '-'},          // subtraction
    {"\\*", '*'},        // multiplication
    {"/", '/'},          // division
    {"\\$\\w+", TK_REG}, // register
    {"[0-9]+", TK_NUM},  // number
    {"==", TK_EQ},       // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg,
                  rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
                pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len,
                    substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                switch (rules[i].token_type) {
                default:
                    tokens[nr_token].type = rules[i].token_type;
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0'; // 字符串结束符
                }

                nr_token++;

                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e,
                   position, "");
            return false;
        }
    }

    return true;
}

bool check_parenthese(int p, int q) {
    int num_left = 0;

    if ((strcmp(tokens[p].str, "(") == 0) ||
        (strcmp(tokens[q].str, ")") == 0)) {
        for (int i = p; i <= q; i++) {
            if (strcmp(tokens[i].str, "(") == 0) {
                num_left++;
            } else if (strcmp(tokens[i].str, ")") == 0) {
                num_left--;
                if (num_left == 0) {
                    // 如果是q之前就到达num_left==0
                    // 说明开头的"("不是与最后的")"配对的
                    return i == q;
                }
            }
        }
    } else {
        return false;
    }

    if (num_left == 0) {
        return true;
    } else {
        return false;
    }
}

word_t find_op(int p, int q) {
    int num_left = 0;
    int low_op = 0; // 最低优先级
    int tmp_op = 0; // 当前op的优先级
    word_t pos = 0;

    for (int i = p; i <= q; i++) {
        if (tokens[i].type == TK_NUM) {
            continue;
        } else if (tokens[i].type == '(') {
            num_left++;
        } else if (tokens[i].type == ')') {
            if (num_left == 0) {
                return -1;
            } else {
                num_left--;
            }
        } else if (num_left != 0) {
            continue;
        } else if (tokens[i].type == '+' || tokens[i].type == '-') {
            tmp_op = 2;

            if (tmp_op >= low_op) {
                low_op = tmp_op;
                pos = i;
            }
        } else if (tokens[i].type == '*' || tokens[i].type == '/') {
            tmp_op = 1;

            if (tmp_op >= low_op) {
                low_op = tmp_op;
                pos = i;
            }
        }
    }

    if (num_left > 0) {
        return -1;
    } else {
        return pos;
    }
}

word_t eval(int p, int q, bool *success) {
    *success = true;
    if (p > q) {
        *success = false;
        return 0;
    } else if (p == q) {
        // 单token，该token必须为数字
        if (tokens[p].type == TK_NUM) {
            return atoi(tokens[p].str);
        } else {
            *success = false;
            return 0;
        }
    } else if (check_parenthese(p, q) == true) {
        *success = true;
        return eval(p + 1, q - 1, success);
    } else {
        word_t op = find_op(p, q);
        word_t val1 = eval(p, op - 1, success);
        word_t val2 = eval(op + 1, q, success);

        switch (tokens[op].type) {
        case '+':
            return val1 + val2;
        case '-':
            return val1 - val2;
        case '*':
            return val1 * val2;
        case '/':
            return val1 / val2;
        default:
            *success = false;
            assert(0);
        }
        return 0;
    }

    return 0;
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    word_t ret = 0;
    ret = eval(0, nr_token - 1, success);

    return ret;
}

char *test_buf;
static int index_buf = 0;

static uint32_t choose(uint32_t n) {
    // 生成一个小于n的随机数
    uint32_t ret = rand() % n;

    return ret;
}

static void gen(char c) {
    test_buf[index_buf] = c;
    index_buf++;
}

static void gen_num() {
    char str[128];
    uint32_t num = rand() % 100;

    sprintf(str, "%d", num);

    strncpy(test_buf + index_buf, str, strlen(str));
    index_buf += strlen(str);
}

static void gen_rand_op() {
    char ops[4] = {'+', '-', '*', '/'};
    int index = choose(4);

    test_buf[index_buf] = ops[index];
    index_buf++;
}

void gen_rand_expr() {
    switch (choose(3)) {
    case 0:
        gen_num();
        break;
    case 1:
        gen('(');
        gen_rand_expr();
        gen(')');
        break;
    default:
        gen_rand_expr();
        gen_rand_op();
        gen_rand_expr();
        break;
    }
}
