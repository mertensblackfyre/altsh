#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/utils.h"

#include "../include/builtin.h"

#define SIZE 1024

int main(void) {

  char *input;

  do {

    char *buffer = malloc(sizeof(char *) * SIZE);

    if (getcwd(buffer, SIZE) == NULL) {
      fprintf(stderr, "error: Could not get path");
      exit(EXIT_FAILURE);
    };

    printf("$ ");

    input = utils_get_input();

    if (builtin_check(input)) {
      continue;
    };
    utils_execute_program(input);

    free(buffer);
    free(input);
  } while (true);

  return EXIT_SUCCESS;
};
