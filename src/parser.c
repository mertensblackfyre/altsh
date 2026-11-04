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
  char *str = calloc(len + 1, sizeof(char));

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
  char *str = calloc(len + 1, sizeof(char));
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
  char *str = calloc(len + 1, sizeof(char));

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

  token.literal = calloc(strlen(str) + 1, sizeof(char));
  strncpy(token.literal, str, strlen(str));
  token.literal[strlen(str)] = '\0';
  free(str);
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

  parsed_input *p_input = calloc(strlen(input) + 1, sizeof(parsed_input));

  if (p_input == NULL) {
    fprintf(stderr,
            "parser_scanner(): memory allocation failed (p_input is NULL)\n");
    return NULL;
  }

  p_input->argv = calloc(strlen(input) + 1, sizeof(char *));
  p_input->argc = 0;
  p_input->has_operator = false;
  p_input->op.file_name = calloc(strlen(input), sizeof(char));
  p_input->op.op = calloc(3, sizeof(char));

  if (p_input->argv == NULL || p_input->op.file_name == NULL ||
      p_input->op.op == NULL) {

    fprintf(stderr, "%s: memory allocation failed \n", __func__);
    return NULL;
  }

  if (strlen(input) == 0) {
    helper_free_parsed_input(p_input);
    return p_input;
  };

  scanner *s = (scanner *)calloc(20, sizeof(scanner));
  if (s == NULL) {

    helper_free_parsed_input(p_input);
    fprintf(stderr, "%s: memory allocation failed \n", __func__);
    return NULL;
  }
  s->start = input;
  s->current = input;

  int pos = -1;
  for (int i = 0; i < strlen(input); i++) {

    token token = parser_scan_token(strlen(input), s);

    if (token.type == TOKEN_EOF)
      break;

    if (token.type == TOKEN_COMMAND) {
      p_input->cmd = calloc(strlen(token.literal) + 1, sizeof(char));

      if (p_input->cmd == NULL) {

        helper_free_parsed_input(p_input);
        fprintf(stderr, "%s: memory allocation failed \n", __func__);
        return NULL;
      }

      strncpy(p_input->cmd, token.literal, strlen(token.literal));
      p_input->cmd[strlen(token.literal)] = '\0';
    }

    if (!p_input->has_operator && token.literal != NULL &&
        token.type == TOKEN_ARGUMENTS && strchr(token.literal, '>') != NULL) {
      p_input->has_operator = true;

      strncpy(p_input->op.op, token.literal, strlen(token.literal));
      continue;
    }

    if (p_input->has_operator) {
      strncpy(p_input->op.file_name, token.literal, token.length);
      continue;
    }

    p_input->argv[i] = calloc(strlen(token.literal) + 1, (sizeof(char)));
    if (p_input->argv[i] == NULL) {
      fprintf(stderr, "%s: memory allocation failed \n", __func__);
      helper_free_parsed_input(p_input);
      free(s);
      return NULL;
    }

    strncpy(p_input->argv[i], token.literal, strlen(token.literal));
    p_input->argc++;
    free(token.literal);
  }

  free(s);
  return p_input;
}
