#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/args.h"
#include "../include/helper.h"

static token handle_commands(int len, scanner *s, token_type type) {

  int index = 0;
  char *str = malloc(len);

  // printf("current-> %c\n", *s->current);
  // printf("start-> %c\n", s->current[-1]);

  if (s->current[-1] == '\\') {
    str[index++] = *s->current;
    s->current++;
  } else {
    str[index++] = s->current[-1];
  }

  while (*s->current != '\'' && *s->current != ' ' && *s->current != '\t' &&
         *s->current != '\r' && *s->current != '\0') {

    if (*s->current == '\\' && *(s->current + 1) != '\0') {
      s->current++;
      str[index++] = *s->current;
      s->current++;
      continue;
    }

    str[index++] = *s->current;
    s->current++;
  }

  if (*s->current == '\'' && *(s->current + 1) != '\0' &&
      *(s->current + 1) == '\'') {
    s->current += 2;
    while (*s->current != ' ' && *s->current != '\t' && *s->current != '\r' &&
           *s->current != '\0') {

      if (*s->current == '\\' && *(s->current + 1) != '\0') {
        s->current++;
        str[index++] = *s->current;
        s->current++;
        continue;
      }

      str[index++] = *s->current;
      s->current++;
    }
  }

  if (*s->current == '\0') {
    str[index++] = '\0';
    s->pos++;
    return args_token_make(str, s, type);
  }

  s->pos++;
  s->current++;
  str[index++] = '\0';
  helper_skip_whitespace(s);
  return args_token_make(str, s, type);
};

static token handle_double_qoutes(int len, scanner *s) {

  int index = 0;
  char *str = malloc(len);
  while (*s->current != '"' && *s->current != '\0') {
    str[index++] = *s->current;
    if (*s->current == '\\') {
      str[index++] = *++s->current;
      continue;
    };
    s->current++;
  }
  if (*(s->current + 1) != '\0' && *(s->current + 1) == '"') {
    s->current += 2;
    while (*s->current != '"' && *s->current != '\0') {
      str[index++] = *s->current;
      s->current++;
    }
  }

  if (*(s->current + 1) != '\0' && isspace(*(s->current + 1)) == 0) {
    s->current++;
    while (*s->current != '"' && *s->current != '\0') {
      str[index++] = *s->current;
      s->current++;
    }
  }

  s->pos++;
  s->current++;

  str[index++] = '\0';
  helper_skip_whitespace(s);
  return args_token_make(str, s, TOKEN_DOUBLE_QUOTE);
}

static token handle_single_qoutes(int len, scanner *s) {

  int index = 0;
  char *str = malloc(len);

  while (*s->current != '\'' && *s->current != '\0') {
    str[index++] = *s->current;
    s->current++;
  }
  if (*(s->current + 1) != '\0' && *(s->current + 1) == '\'') {
    s->current += 2;
    while (*s->current != '\'' && *s->current != '\0') {
      str[index++] = *s->current;
      s->current++;
    }
  }
  s->pos++;
  s->current++;

  str[index++] = '\0';
  helper_skip_whitespace(s);
  return args_token_make(str, s, TOKEN_SINGLE_QUOTE);
}

token args_token_make(char *str, scanner *s, token_type type) {
  token token;
  token.type = type;
  token.start = s->start;
  token.length = (int)(s->current - s->start);
  token.pos = s->pos;

  if (strlen(str) == 0)
    return token;

  token.literal = malloc(strlen(str));
  strncpy(token.literal, str, token.length);
  token.literal[token.length] = '\0';
  return token;
};

token args_token_scan(int len, scanner *s) {
  s->start = s->current;

  if (*s->current == '\0')
    return args_token_make("", s, TOKEN_EOF);

  s->current++;
  char c = s->current[-1];

  if (s->pos == 0) {
    return handle_commands(len, s, TOKEN_COMMAND);
  }

  switch (c) {
  case '"':
    return handle_double_qoutes(len, s);
  case '\'':
    return handle_single_qoutes(len, s);
  default:
    return handle_commands(len, s, TOKEN_ARGUMENTS);
  };
  return args_token_make("", s, TOKEN_EOF);
};

char **args_scanner(char *input) {

  char **args = malloc(sizeof(token) * strlen(input));
  if (strlen(input) == 0) {
    fprintf(stderr, "No input provided\n");
    return args;
  };

  scanner *s = (scanner *)malloc(10 * sizeof(scanner));

  if (s == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  s->start = input;
  s->current = input;

  int pos = -1;

  for (int i = 0; i < strlen(input); i++) {
    token token = args_token_scan(strlen(input), s);

    if (token.type == TOKEN_EOF)
      break;

    printf("%s\n", token.literal);
    // args[i] = malloc(token.length);
    // strncpy(args[i], token.literal, token.length);
  }

  return args;
}
