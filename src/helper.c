#include "../include/parser.h"
#include <stdbool.h>
#include <stdio.h>
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
  FILE *fp = fopen(file_name, mode);

  if (fp == NULL) {
    fprintf(stderr, "error opening file");
    return;
  }

  fprintf(fp, "%s", content);
  fclose(fp);
}
