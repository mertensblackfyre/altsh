#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/builtin.h"
#include "../include/helper.h"
#include "../include/parser.h"
#include "../include/utils.h"

#define SIZE 1024

const char *builtin[] = {"echo", "type", "exit", "pwd", "cd"};

bool builtin_check(char *input) {

  size_t size = strlen(input);
  parsed_input *p_input = malloc(sizeof(parsed_input) * size);
  if (p_input == NULL) {
    perror("failed allocation: builtin_check()");
    return false;
  }

  p_input = parser_scanner(input);
  if (p_input == NULL) {
    perror("failed init: builtin_check()");
    return false;
  }

  char *cmd = p_input->cmd;

  if (cmd == NULL) {
    perror("first value is empty: builtin_check()");
    return false;
  }

  if (strncmp("exit", cmd, 4) == 0) {
    if (strlen(cmd) == 0) {
      builtin_exit(1);
      return true;
    }

    int n = atoi((char *)p_input->argv[1]);
    builtin_exit(n);
    return true;
  };

  if (strncmp("echo", cmd, 4) == 0) {
    builtin_echo(p_input);
    return true;
  };

  if (strncmp("pwd", cmd, 3) == 0) {
    builtin_pwd();
    return true;
  };

  if (strncmp("cd", cmd, 2) == 0) {
    builtin_cd(p_input);
    return true;
  };
  if (strncmp("type", cmd, 4) == 0) {
    if (builtin_type(p_input->argv[1]))
      printf("%s is a shell builtin\n", p_input->argv[1]);
    else
      utils_get_path(p_input->argv[1]);
    return true;
  };

  return false;
};

void builtin_cd(parsed_input *args) {

  if (args->argc == 1) {
    const char *env_variable = "HOME";
    char *value = getenv(env_variable);
    char *path = value;

    int err = chdir(path);
    return;
  };

  char *new_path = malloc(sizeof(char *) * SIZE);
  char *buffer = malloc(sizeof(char *) * SIZE);
  char *path = args->argv[1];

  getcwd(buffer, SIZE);

  if (strstr(path, "../") != NULL) {
    int x = helper_count_substrings(path);
    char *tmp = path;
    path = helper_backtrack_path(buffer, x);
    strncat(path, tmp, strlen(tmp));
  };

  if (path[0] == '~') {
    const char *env_variable = "HOME";
    char *value = getenv(env_variable);
    path = value;
  };

  if (path[0] == '.') {
    for (int i = 1; i < strlen(path); ++i)
      new_path[i - 1] += path[i];

    strcat(buffer, new_path);
    path = buffer;
  }

  int err = chdir(path);

  if (err == -1) {
    if (errno == ENOENT)
      fprintf(stderr, "Error: directory specified in path does not exist\n");

    if (errno == EACCES)
      fprintf(stderr, "Error: permission was denied\n");

    if (errno == ENOTDIR)
      fprintf(stderr, "Error: path is not a directory\n");
  }
  return;
};
void builtin_pwd() {
  char *buffer = malloc(sizeof(char *) * SIZE);
  if (getcwd(buffer, SIZE) == NULL)
    printf("error: Could not get path\n");

  printf("%s\n", buffer);
};
void builtin_exit(int status) {
  exit(1);
  return;
}
void builtin_echo(parsed_input *data) {
  for (size_t i = 1; i < data->argc; ++i)
    printf("%s ", data->argv[i]);

  printf("\n");
};

bool builtin_type(char *arg) {
  size_t size = sizeof(builtin) / sizeof(builtin[0]);

  for (size_t i = 0; i < size; ++i) {
    size_t arg_size = sizeof(&arg) / sizeof(arg[0]);
    size_t builtin_size = sizeof(&builtin[i]) / sizeof(builtin[i][0]);
    if (arg_size == builtin_size) {
      if (strncmp(helper_strtrim(arg), builtin[i], arg_size) == 0) {
        return true;
      };
    }
  };

  return false;
};
