#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "builtin.h"
#include "utils.h"

// Exécution d'un builtin avec redirections
static int exec_builtin(command_t *cmd) {
    if (!cmd || !cmd->argv[0]) return -1;

    for (int i = 0; builtins[i].name; ++i) {
        if (strcmp(cmd->argv[0], builtins[i].name) == 0) {

            if (cmd->redir_out && cmd->outfile) {
                buildtin_redirect_output(cmd->outfile, 0);
            }


            if (cmd->redir_append && cmd->outfile) {
                buildtin_redirect_output(cmd->outfile, 1);
            }


            if (cmd->redir_in && cmd->infile) {
                builtin_redirect_input(cmd->infile);
            }


            if (cmd->heredoc && cmd->heredoc_content) {
                builtin_heredoc_input(cmd->heredoc_content);
            }
   
            return builtins[i].function(cmd->argv);
        }
    }

    return -1; 
}

// Exécution d'une commande système
static int exec_simple(command_t *cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        if (cmd->redir_out) {
            buildtin_redirect_output(cmd->outfile, 0);
        }
            
        if (cmd->redir_append) {
            buildtin_redirect_output(cmd->outfile, 1);
        }
            
        if (cmd->redir_in) {
            builtin_redirect_input(cmd->infile);
        }
            
        if (cmd->heredoc) {
             builtin_heredoc_input(cmd->heredoc_content);
        }
           
        execvp(cmd->argv[0], cmd->argv);
        perror("exec");

        exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}

// Exécution générale
int execute(command_t *cmd) {
    if (!cmd) return -1;

    // Vérifier si builtin
    int status = exec_builtin(cmd);
    if (status != -1) {
        return status;
    }
        

    if (cmd->background) {
        pid_t pid = fork();

        if (pid == 0) {
            exec_simple(cmd);
            exit(0);
        }

        printf("[Process %d lancé en arrière-plan]\n", pid);

        return 0;
    }

    return exec_simple(cmd);
}