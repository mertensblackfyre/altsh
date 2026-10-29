#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/helper.h"
#include "../include/parser.h"

static token handle_single_qoutes(int len, scanner *s);
static token handle_double_qoutes(int len, scanner *s);

static token handle_commands(int len, scanner *s, token_type type) {

  int index = 0;
  char *str = malloc(len);

  if (s->current[-1] == '\'') {
    return handle_single_qoutes(len, s);
  }
  if (s->current[-1] == '"') {
    return handle_double_qoutes(len, s);
  }

  if (s->current[-1] == '\\') {
    str[index++] = *s->current;
    s->current++;
  } else {
    str[index++] = s->current[-1];
  }

  while (*s->current != ' ' && *s->current != '\t' && *s->current != '\r' &&
         *s->current != '\0') {

    if (*s->current == '\\' && *(s->current + 1) != '\0') {
      s->current++;
      str[index++] = *s->current;
      s->current++;
      continue;
    }

    if (*s->current == '"' && *(s->current + 1) != '\0' &&
        *(s->current + 1) == '"') {
      s->current += 2;
    }

    if (*s->current == '"' && *(s->current + 1) != '\0') {
      s->current++;
      while (*s->current != '"' && *s->current != '\0') {
        char next = s->current[1];
        if (*s->current == '\\' &&
            (next == '"' || next == '\\' || next == '$' || next == '`' ||
             next == '\n')) {

          s->current++;
          str[index++] = *s->current;
          s->current++;
          continue;
        };
        str[index++] = *s->current;
        s->current++;
      }
      s->current++;
      continue;
    }

    if (*s->current == '\'' && *(s->current + 1) != '\0' &&
        *(s->current + 1) == '\'') {
      s->current += 2;
    }
    if (*s->current == '\'' && *(s->current + 1) != '\0') {
      s->current++;
      while (*s->current != '\'' && *s->current != '\0') {

        str[index++] = *s->current;
        s->current++;
      }
      s->current++;
      continue;
    }

    str[index++] = *s->current;
    s->current++;
  }

  if (*s->current == '\0') {
    str[index++] = '\0';
    s->pos++;
    return parser_create_token(str, s, type);
  }

  s->pos++;
  s->current++;
  str[index++] = '\0';
  helper_skip_whitespace(s);
  return parser_create_token(str, s, type);
};

static token handle_double_qoutes(int len, scanner *s) {

  int index = 0;
  char *str = malloc(len);
  while (*s->current != '"' && *s->current != '\0') {
    char next = s->current[1];
    if (*s->current == '\\' && (next == '"' || next == '\\' || next == '$' ||
                                next == '`' || next == '\n')) {

      s->current++;
      str[index++] = *s->current;
      s->current++;
      continue;
    };

    str[index++] = *s->current;
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
      char next = s->current[1];
      if (*s->current == '\\' && (next == '"' || next == '\\' || next == '$' ||
                                  next == '`' || next == '\n')) {

        s->current++;
        str[index++] = *s->current;
        s->current++;
        continue;
      }
      str[index++] = *s->current;
      s->current++;
    }
  }

  s->pos++;
  s->current++;

  str[index++] = '\0';
  helper_skip_whitespace(s);
  return parser_create_token(str, s, TOKEN_DOUBLE_QUOTE);
}

static token handle_single_qoutes(int len, scanner *s) {

  int index = 0;
  char *str = malloc(len);

  while (*s->current != '\'' && *s->current != '\0') {
    if (*s->current == '"') {
      while (*s->current != '"' && *s->current != '\0') {
        str[index++] = *s->current;
        s->current++;
      }
    }

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
  return parser_create_token(str, s, TOKEN_SINGLE_QUOTE);
}

token parser_create_token(char *str, scanner *s, token_type type) {
  token token;
  token.type = type;
  token.start = s->start;
  token.length = (int)(s->current - s->start);
  token.pos = s->pos;

  if (strlen(str) == 0)
    return token;

  token.literal = malloc(strlen(str) * 2);
  strncpy(token.literal, str, token.length);
  token.literal[token.length] = '\0';
  return token;
};

token parser_scan_token(int len, scanner *s) {
  s->start = s->current;

  if (*s->current == '\0')
    return parser_create_token("", s, TOKEN_EOF);

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
  return parser_create_token("", s, TOKEN_EOF);
};

parsed_input *parser_scanner(char *input) {

  parsed_input *p_input = malloc(sizeof(parsed_input) * strlen(input) + 1);
  p_input->argv = malloc(sizeof(token) * strlen(input) + 1);
  p_input->argc = 0;

  if (strlen(input) == 0) {
    return p_input;
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
    token token = parser_scan_token(strlen(input), s);

    if (token.type == TOKEN_EOF)
      break;

    if (token.type == TOKEN_COMMAND) {
      p_input->cmd = malloc(token.length);
      strncpy(p_input->cmd, token.literal, token.length);
    }

    if (token.literal != NULL && strcmp(token.literal, ">") == 0) {
        p_input->has_operator = true;
    }else{
        p_input->has_operator = false;
    }

    p_input->argv[i] = malloc(token.length);
    strncpy(p_input->argv[i], token.literal, token.length);
    p_input->argc++;
  }

  return p_input;
}
