#ifndef TERMINUS_H
#define TERMINUS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>   // fork()
#include <sys/wait.h> // wait()

#include "typedef.h"

// SECTION - Directives de préprocesseur
#define PROJECT_NAME "Terminus"
#define DELIMITER "\n\t "

#define BATCH_PARAMETER "-c"
#define BATCH_PARAMETER_LONG "--command"
#define HELP_PARAMETER_LONG "--help"

// Couleurs ANSI
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[1;32m"
#define COLOR_WHITE "\x1b[1;37m"

// SECTION - Prototypes
void displayPrompt(void);
char *getUserInput(void);
char **splitInput(char *string, bool *is_background);
int executeCommands(char **arguments, bool is_background, int input_fd, int output_fd);
int executePipeline(char **piped_commands);
char *get_history_path();
void display_history();
void save_command_to_history(char *command);
char *replace_heredoc_arg(char *raw_command, const char *delimiter, const char *temp_filename);

#endif