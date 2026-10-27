#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

typedef enum {
  TOKEN_COMMAND,
  TOKEN_ARGUMENTS,
  TOKEN_SINGLE_QUOTE,
  TOKEN_DOUBLE_QUOTE,
  TOKEN_IGNORE,
  TOKEN_EOF
} token_type;

typedef struct {
  const char *start;
  const char *current;
  int pos;
} scanner;

typedef struct {
  token_type type;
  const char *start;
  char *literal;
  int length;
  int pos;
} token;

char **parser_scanner(char *input);
token parser_scan_token(int len,scanner *s);

token parser_create_token(char *str, scanner *s, token_type type);
#endif
