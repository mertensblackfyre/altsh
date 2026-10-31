#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>
#include "../include/parser.h"

extern const char *builtin[];

bool builtin_check(char *input);
bool builtin_type(char *arg);
void builtin_exit(int status);
void builtin_echo(parsed_input *data);
char *builtin_pwd();
void builtin_cd(parsed_input *args);

#endif
