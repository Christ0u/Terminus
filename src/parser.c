#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "utils.h"

// Découpe basique en tokens
static char **split_tokens(char *line) {
    size_t size = 64;
    size_t i = 0;

    char **tokens = malloc(sizeof(char*) * size);
    if (!tokens) {
        return NULL;
    }

    char *tok = strtok(line, " \t\n");
    while (tok) {
        tokens[i++] = tok;

        if (i >= size) {
            size *= 2;
            tokens = realloc(tokens, sizeof(char*) * size);

            if (!tokens) {
                return NULL;
            }
        }

        tok = strtok(NULL, " \t\n");
    }

    tokens[i] = NULL;

    return tokens;
}


// ----------------------------------------------------
//   Fonction qui retire les opérateurs des argv
//   et stocke correctement les fichiers / contenu
// ----------------------------------------------------
void parse_redirections(command_t *cmd) {
    if (!cmd || !cmd->argv) {
        return;
    } 

    int write_idx = 0;
    for (int i = 0; cmd->argv[i]; i++) {
        if (strcmp(cmd->argv[i], ">") == 0 && cmd->argv[i+1]) {
            cmd->redir_out = 1;
            cmd->outfile = strdup(cmd->argv[i+1]);
            i++;
            continue;
        }

        if (strcmp(cmd->argv[i], ">>") == 0 && cmd->argv[i+1]) {
            cmd->redir_append = 1;
            cmd->outfile = strdup(cmd->argv[i+1]);
            i++;
            continue;
        }

        if (strcmp(cmd->argv[i], "<") == 0 && cmd->argv[i+1]) {
            cmd->redir_in = 1;
            cmd->infile = strdup(cmd->argv[i+1]);
            i++;
            continue;
        }

        if (strcmp(cmd->argv[i], "<<") == 0 && cmd->argv[i+1]) {
            cmd->heredoc = 1;
            cmd->heredoc_content = strdup(cmd->argv[i+1]);
            i++;
            continue;
        }

        cmd->argv[write_idx++] = cmd->argv[i];
    }

    cmd->argv[write_idx] = NULL;
}



command_t *parse_line(char *line) {

    command_t *cmd = malloc(sizeof(command_t));
    if (!cmd) {
        return NULL;
    }

    memset(cmd, 0, sizeof(command_t));

    // Tokenisation
    cmd->argv = split_tokens(line);
    if (!cmd->argv) {
        free(cmd);
        return NULL;
    }

    // Gestion des redirections + nettoyage argv
    parse_redirections(cmd);

    return cmd;
}


// ----------------------------------------------------
//   Debug
// ----------------------------------------------------
void debug_print_command(command_t *cmd) {
    printf("=== DEBUG COMMAND ===\n");

    for (int i = 0; cmd->argv && cmd->argv[i]; i++) {
        printf("argv[%d] = '%s'\n", i, cmd->argv[i]);
    }

    printf("redir_out      = %d\n", cmd->redir_out);
    printf("redir_append   = %d\n", cmd->redir_append);
    printf("redir_in       = %d\n", cmd->redir_in);
    printf("heredoc        = %d\n", cmd->heredoc);
    printf("outfile        = %s\n", cmd->outfile ? cmd->outfile : "(null)");
    printf("infile         = %s\n", cmd->infile ? cmd->infile : "(null)");
    printf("heredoc_content= %s\n", cmd->heredoc_content ? cmd->heredoc_content : "(null)");
    printf("======================\n");
}
