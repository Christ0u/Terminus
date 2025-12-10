#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

typedef int (*builtin_func)(char **arguments);

typedef struct {
    const char *name;
    builtin_func function;
} builtin_cmd;

extern builtin_cmd builtins[];

int builtin_exit(char **arguments);
int builtin_pwd(char **arguments);
int builtin_echo(char **arguments);
int builtin_cd(char **arguments);

int buildtin_redirect_output(const char *filename, int append);
int builtin_redirect_input(const char *filename);
int builtin_heredoc_input(const char *content);
int builtin_create_pipe(int pipefd[2]);
int builtin_execute_and(int exitStatus, void (*nextCommand)(void));

#endif
