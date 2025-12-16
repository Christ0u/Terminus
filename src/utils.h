#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void print_error(char *message);
char *read_heredoc_content(const char *delimiter);
char **split_pipes(char *string);
char *find_command_path(const char *cmd);
char **split_and_commands(char *line);
char *get_user_input(void);
char **split_input(char *string, bool *is_background);
char *get_history_path();
void display_history();
void save_command_to_history(char *command);
char *replace_heredoc_arg(char *raw_command, const char *delimiter, const char *temp_filename);
void sigchld_handler(int sig);

// NOTE - Fonctions de debug
void debug_print_command(command_t *cmd);
void display_arguments(int argc, char **argv);

#endif