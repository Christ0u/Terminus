#include "Terminus.h"

// getcwd wrapper
void getCurrentWorkingDirectory(char *buffer, size_t size)
{
    if (NULL == getcwd(buffer, size))
    {
        perror("getcwd");
    }
}

void printError(char *message)
{
    printf(COLOR_RED "ERR : %s\n" COLOR_RESET, message);
}

// NOTE - Fonctions temporaires
void displayArguments(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("Argument n°%d : ", i);

        for (int j = 0; argv[i][j] != '\0'; j++)
        {
            printf("%c", argv[i][j]);
        }

        printf("\n");
    }
}