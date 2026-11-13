#include "../include/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *helper_strtrim(char *str) {

  if (strlen(str) == 0)
    return str;
  char *end;
  while (*str == ' ' || *str == '\t' || *str == '\r')
    str++;
  if (*str == 0)
    return str;
  end = str + strlen(str) - 1;
  while (*end == ' ' || *end == '\t' || *end == '\r')
    end--;
  *(end + 1) = 0;
  return str;
}

void helper_skip_whitespace(scanner *s) {
  if (s == NULL || s->current == NULL)
    return;

  for (;;) {
    char c = *s->current;
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      s->current++;
      break;
    default:
      return;
    }
  }
}

bool helper_ispace(scanner *s) {
  char c = *s->current;
  return c == ' ' || c == '\t' || c == '\r' || c == '\0';
}

char *helper_backtrack_path(char *s, int count) {
  size_t len = strlen(s);
  for (int i = len; i > 0; --i) {
    if (s[i] == '/')
      count--;
    if (count > 0)
      s[i] = '\0';
  };
  return s;
};

int helper_count_substrings(char *s) {

  size_t size = strlen(s);
  int count = 0;

  while ((s = strstr(s, "../")) != NULL) {
    count++;
    memmove(s, s + 3, strlen(s + 3) + 1);
  };
  return count;
};

void helper_write(char *file_name, char *content, char *mode) {

  char *m = calloc(3, sizeof(char));

  if (m == NULL) {
    fprintf(stderr, "altsh: error allocating \n");
    return;
  }

  if (strcmp(mode, ">") == 0)
    strncpy(m, "w+", 2);

  if (strcmp(mode, ">>") == 0)
    strncpy(m, "a+", 2);

  if (strlen(m) == 0) {
    fprintf(stderr, "altsh: error redirecting \n");
    free(m);
    return;
  }

  FILE *fp = fopen(file_name, m);

  if (fp == NULL) {
    fprintf(stderr, "altsh: %s: No such file or directory\n", file_name);
    return;
  }

  fprintf(fp, "%s", content);
  fclose(fp);
  free(m);
}

void helper_free_parsed_input(parsed_input *p) {
  free(p->op.op);
  free(p->op.file_name);
  free(p->argv);
  free(p->cmd);
  free(p);
}
