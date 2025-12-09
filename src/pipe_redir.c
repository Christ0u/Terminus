#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "pipe_redir.h"

int exec_with_redir_or_pipe(command_t *cmd) {
    int fd;

    pid_t pid = fork();
    if (pid == 0) {

        // Redirection >
        if (cmd->redir_out) {
            fd = open(cmd->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Redirection >>
        if (cmd->redir_append) {
            fd = open(cmd->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Redirection <
        if (cmd->redir_in) {
            fd = open(cmd->filename, O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }


        execvp(cmd->argv[0], cmd->argv);
        perror("exec");
        exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}
