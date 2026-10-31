#ifndef UTILS_H
#define UTILS_H

#include "./parser.h"

char *utils_get_input();
void utils_get_path(parsed_input *p_input);
char *utils_get_command(char *input);
void utils_execute_command(char *cmd);
void utils_execute(char *string);
void utils_execute_program(char *cmd);
void utils_append_file(const char *filename, const bool append,
                       const bool overwrite);

#endif
