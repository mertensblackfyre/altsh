#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/utils.h"
#include "../include/helper.h"

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
  close(fd[1]);

  if (cpid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    close(fd[0]);
    close(fd[1]);
    char *args[] = {"which", cmd, NULL};
    execvp("which", args);
  } else {
    int status = 0;
    size_t p = 0;
    ssize_t _size;
    while ((_size = read(fd[0], buffer + p, BUFFER_SIZE - 1 - p)) != 0) {
      p += _size;
      if (p >= BUFFER_SIZE - 1)
        break;
    }
    buffer[p] = '\0';
    close(fd[0]);
    waitpid(cpid, &status, 0);
    if (_size != 0) {
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

  int fd[2];

  int fd_pipe = pipe(fd);
  pid_t cpid = fork();
  close(fd[1]);

  if (cpid == -1) {
    perror("fork()");
    return;
  };


  if (cpid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    close(fd[0]);
    execvp(cmd, p_input->argv);
    printf("%s: command not found\n", cmd);
  } else {
    size_t p = 0;
    ssize_t _size;
    while ((_size = read(fd[0], buffer + p, BUFFER_SIZE - 1 - p)) != 0) {
      p += _size;
      if (p >= BUFFER_SIZE - 1)
        break;
    }
    buffer[p] = '\0';
    close(fd[0]);
    int status;
    waitpid(cpid, &status, 0);

    if (p_input->has_operator) {
        helper_write(p_input->argv[p_input->argc - 1],buffer, "w" );
        return;
    }
    printf("%s", buffer);
  };
};
