#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "exec.h"
#include "pipe_redir.h"
#include "utils.h"

static int exec_simple(command_t *cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        perror("exec");
        exit(1);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

int execute(command_t *cmd) {
    // pipe et redirection ?
    if (cmd->has_pipe || cmd->redir_in || cmd->redir_out || cmd->redir_append || cmd->heredoc)
        return exec_with_redir_or_pipe(cmd);

    if (cmd->background) {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(cmd->argv[0], cmd->argv);
            perror("exec");
            exit(1);
        }
        printf("[Process %d lancé en arrière-plan]\n", pid);
        return 0;
    }

    return exec_simple(cmd);
}
