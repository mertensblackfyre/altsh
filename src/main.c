#include "../include/utils.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    // if (builtin_check(input)) {
    //    continue;
    //  };

   // args_scanner(input);

    //utils_execute_program(input);
    free(buffer);
    free(input);
  } while (true);

  // args_parse_arguments(input);
  return EXIT_SUCCESS;
};
