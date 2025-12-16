#ifndef BUILTIN_H
#define BUILTIN_H

#include "terminus.h"
#include "alias.h"

typedef int (*builtin_func)(char **arguments);

typedef struct
{
    const char *name;
    builtin_func function;
} builtin_cmd;

extern builtin_cmd builtins[];

void init_environment();
int set_env_var(const char *var_assignment);
void display_history();
int builtin_export(char **args);
int builtin_unset(char **args);
int builtin_exit(char **arguments);
int builtin_pwd(char **arguments);
int builtin_echo(char **arguments);
int builtin_cd(char **arguments);
int buildtin_redirect_output(const char *filename, int append);
int builtin_redirect_input(const char *filename);
int builtin_heredoc_input(const char *content);
int builtin_history(char **args);
int builtin_alias(char **args);
int builtin_unalias(char **args);

#endif
