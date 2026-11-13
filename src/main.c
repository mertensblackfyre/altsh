
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/builtin.h"
#include "../include/utils.h"
#include <readline/history.h>
#include <readline/readline.h>

#define SIZE 1024

char *get_history_path(void) {
  char *home = getenv("HOME");
  if (!home)
    return NULL;

  static char path[1024];
  snprintf(path, sizeof(path), "%s/.altsh_history", home);
  return path;
}

int main(void) {
  char *input;
  char *history_file = get_history_path();
  if (history_file) {
    read_history(history_file);
    stifle_history(1000);
  }

  do {

    char *buffer = malloc(sizeof(char *) * SIZE);
    if (buffer == NULL) {
      fprintf(stderr, "failed buffer allocation");
      return EXIT_FAILURE;
    }

    input = readline("$ ");

    if (input == NULL || strlen(input) == 0)
      continue;

    if (*input)
      add_history(input);

    if (builtin_check(input))
      continue;

    utils_execute_program(input);

    free(buffer);
    free(input);
  } while (true);

  if (history_file)
    write_history(history_file);

  return EXIT_SUCCESS;
};
