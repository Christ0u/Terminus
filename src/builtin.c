#include "terminus.h"

int builtin_exit(char **arguments)
{
    // printf("Built-in exit called\n");
    (void)arguments;

    exit(EXIT_SUCCESS);
}

int builtin_pwd(char **arguments)
{
    //printf("Built-in pwd called\n");
    (void)arguments;

    char *currentWorkingDirecetory;
    currentWorkingDirecetory = getcwd(NULL, 0);

    if (currentWorkingDirecetory == NULL)
    {
        perror("getcwd() failed");
        return EXIT_FAILURE;
    }
    else
    {
        printf("%s\n", currentWorkingDirecetory);
    }

    return EXIT_SUCCESS;
}