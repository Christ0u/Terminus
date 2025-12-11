#include "terminus.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int builtin_exit(char **arguments)
{
    (void)arguments;

    exit(EXIT_SUCCESS);
}

int builtin_pwd(char **arguments)
{
    (void)arguments;

    char *currentWorkingDirecetory;
    currentWorkingDirecetory = getcwd(NULL, 0);

    if (currentWorkingDirecetory == NULL)
    {
        perror("getcwd failed");
        return EXIT_FAILURE;
    } else
    {
        printf("%s\n", currentWorkingDirecetory);
    }

    return EXIT_SUCCESS;
}

int builtin_echo(char **arguments)
{
    // Si aucun argument ou un argument vide est précisé
    if (!arguments || !arguments[1])
    {
        printf("\n");
        return EXIT_SUCCESS;
    }

    for (int i = 1; arguments[i] != NULL; ++i)
    {
        printf("%s", arguments[i]);

        if (arguments[i + 1])
        {
            printf(" ");
        }
    }

    printf("\n");

    return EXIT_SUCCESS;
}

int builtin_cd(char **arguments)
{
    char *path;

    // Si aucun argument ou un argument vide est précisé
    if (!arguments || !arguments[1])
    {
        // Récupération du répertoire HOME de l'utilisateur
        path = getenv("HOME");
    } else
    {
        path = arguments[1];
    }

    if (chdir(path) == -1)
    {
        perror("cd failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int buildtin_redirect_output(const char *filename, int append)
{
    int fd;

    if (append) {
        fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
        
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

int builtin_redirect_input(const char *filename)
{
    int fd = open(filename, O_RDONLY);

    if (fd < 0) {
        perror("open");

        return -1;
    }

    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2");
        close(fd);

        return -1;
    }

    close(fd);

    return 0;
}

int builtin_heredoc_input(const char *content)
{
    int fd[2];

    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }

    write(fd[1], content, strlen(content));
    close(fd[1]);             
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);

    return 0;
}


int builtin_create_pipe(int pipefd[2])
{
    if (pipe(pipefd) < 0) {
        perror("pipe");

        return -1;
    }
    
    return 0;
}

int builtin_execute_and(int exitStatus, void (*nextCommand)(void))
{
    if (exitStatus == 0 && nextCommand != NULL) {
        nextCommand();
    }
    
    return 0;
}