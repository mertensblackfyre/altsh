#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/utils.h"

#define BUFFER_SIZE 10240

char *utils_get_input() {
  char *input = malloc(sizeof(char) * 100);
  char *s = fgets(input, 100, stdin);
  input[strlen(input) - 1] = '\0';
  return input;
};

void utils_get_path(char *cmd) {
  int fd[2];
  int fd_pipe = pipe(fd);

  char buffer[BUFFER_SIZE];
  char path[256] = "/usr/bin/";
  strcat(path, cmd);

  pid_t cpid = fork();

  if (cpid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    char *args[] = {"which", cmd, NULL};
    execvp("which", args);
    perror("execvp");
    exit(1);
  } else {
    int status;
    waitpid(cpid, &status, 0);
    size_t size = read(fd[0], buffer, BUFFER_SIZE);
    buffer[size - 1] = '\0';

    close(fd[0]);
    close(fd[1]);

    if (size != 0) {
      printf("%s is in %s\n", cmd, buffer);
      return;
    }
    printf("%s: not found\n", cmd);

    return;
  };
  return;
};

void utils_execute_program(char *input) {

  size_t size = strlen(input);
  parsed_input *p_input = malloc(sizeof(parsed_input) * size);

  if (p_input == NULL) {
    perror("failed allocation: builtin_check()");
    return;
  }

  p_input = parser_scanner(input);

  if (p_input == NULL) {
    perror("failed init: builtin_check()");
    return;
  }

  char *cmd = p_input->cmd;

  char buffer[BUFFER_SIZE];

  // if (args->has_operator) {
  // utils_redirect_stdout(buffer, args);
  // return;
  // }

  int fd[2];

  int fd_pipe = pipe(fd);
  pid_t cpid = fork();

  if (cpid == -1) {
    perror("fork()");
    return;
  };

  if (cpid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    execvp(cmd, p_input->argv);
    printf("%s: command not found\n", cmd);
  } else {
    int status;
    waitpid(cpid, &status, 0);
    size_t size = read(fd[0], buffer, BUFFER_SIZE);
    buffer[size] = '\0';
    close(fd[0]);
    close(fd[1]);
    printf("%s", buffer);
  };
};
