#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "utils.h"

// découpe simple
static char **split_tokens(char *line) {
    size_t bufferSize = 64; 
    char **tokens = malloc(sizeof(char*) * bufferSize);
    int i = 0;
    
    if (!tokens) {
        perror("malloc failed in split_tokens");
        return NULL;
    }

    char *tok = strtok(line, " \t\n");
    while (tok) {
        tokens[i++] = tok;
        
        if (i >= bufferSize) {
            bufferSize *= 2; 
            char **new_tokens = realloc(tokens, sizeof(char*) * bufferSize);

            if (!new_tokens) {
                perror("realloc failed in split_tokens");
                free(tokens); 

                return NULL; 
            }
            tokens = new_tokens;
        }

        tok = strtok(NULL, " \t\n");
    }

    if (i == 0) {
        tokens[0] = NULL;
    } else {
        tokens[i] = NULL; 
    }

    return tokens;
}

// TODO : optimiser car c'est pas Édouard-approved
command_t *parse_line(char *line) {

    command_t *cmd = malloc(sizeof(command_t));
    memset(cmd, 0, sizeof(command_t));

    cmd->argv = split_tokens(line);

    if (cmd->argv == NULL) {
        free(cmd);
        return NULL;
    }

    for (int i = 0; cmd->argv[i]; i++) {
        if (strcmp(cmd->argv[i], "|") == 0) {
            cmd->has_pipe = 1;
        }

        if (strcmp(cmd->argv[i], "<") == 0) {
            cmd->redir_in = 1;
        }

        if (strcmp(cmd->argv[i], ">") == 0)  {
            cmd->redir_out = 1;
        }

        if (strcmp(cmd->argv[i], ">>") == 0) {
            cmd->redir_append = 1;
        }

        if (strcmp(cmd->argv[i], "<<") == 0) {
            cmd->heredoc = 1;
        }

        if (strcmp(cmd->argv[i], "&&") == 0) {
            cmd->has_and = 1;
        }

        if (strcmp(cmd->argv[i], "||") == 0) {
            cmd->has_or = 1;
        }

        if (strcmp(cmd->argv[i], "&") == 0) {
            cmd->background = 1;
        }
    }

    return cmd;
}

void debug_print_command(command_t *cmd) {
    printf("=== DEBUG COMMAND ===\n");

    for (int i = 0; cmd->argv && cmd->argv[i]; i++) {
        printf("argv[%d] = '%s'\n", i, cmd->argv[i]);
    }

    printf("has_pipe = %d\n", cmd->has_pipe);
    printf("redir_in = %d\n", cmd->redir_in);
    printf("redir_out = %d\n", cmd->redir_out);
    printf("redir_append = %d\n", cmd->redir_append);
    printf("heredoc = %d\n", cmd->heredoc);

    printf("has_and = %d\n", cmd->has_and);
    printf("has_or = %d\n", cmd->has_or);
    printf("background = %d\n", cmd->background);

    printf("======================\n");
}
