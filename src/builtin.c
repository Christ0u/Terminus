#include "terminus.h"

int builtin_exit(char **arguments)
{
    // printf("Built-in exit called\n");
    (void)arguments;

    exit(EXIT_SUCCESS);
}

int builtin_pwd(char **arguments)
{
    // printf("Built-in pwd called\n");
    (void)arguments;

    char *currentWorkingDirecetory;
    currentWorkingDirecetory = getcwd(NULL, 0);

    if (currentWorkingDirecetory == NULL)
    {
        perror("getcwd failed");
        return EXIT_FAILURE;
    }
    else
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
    }
    else
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