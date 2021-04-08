#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stdbool.h"
#define MAX_STR_SIZE 10000

typedef struct str {
  char my_string[MAX_STR_SIZE];
  int idx;
} my_arg;

void parse_num(my_arg *arg);
void parse_expr(my_arg *arg);
void parse_paren(my_arg *arg);


void parse_num(my_arg *arg) {
  for (;arg->idx < strlen(arg->my_string); arg->idx++) {
    char c = arg->my_string[arg->idx];
    if (!isdigit(c)) {
      break;
    }
  }
  return;
}

void parse_paren(my_arg *arg) {
  char c = arg->my_string[arg->idx];
  char paren_open = '(';
  if (c != paren_open) {
    printf("Invalid parse!\n");
    return;
  }
  arg->idx += 1;
  parse_expr(arg);
  if (arg->my_string[arg->idx] != ')') {
    return;
  }
  arg->idx += 1;
}

void parse_expr(my_arg *arg) {
  while (arg->idx < strlen(arg->my_string)) {
    char c = arg->my_string[arg->idx];
    if (isdigit(c)) {
      parse_num(arg);
    } else if (c == '+' || c == '-' || c == '*' || c == '/') {
      arg->idx += 1;
    } else if (c == '(') {
      parse_paren(arg);
    } else if (c == ')') {
      break;
    }
    else {
      arg->idx += 1;
    }
  }
}

int main(int argc, char *argv[]) {
    my_arg arg;
    strcpy(arg.my_string, argv[1]);
    arg.idx = 0;
    parse_expr(&arg);
}