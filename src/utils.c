#include "terminus.h"

// getcwd wrapper
void getCurrentWorkingDirectory(char *buffer, size_t size)
{
    if (NULL == getcwd(buffer, size))
    {
        perror("getcwd");
    }
}

// malloc wrapper
void *createMemoryAllocation(size_t size)
{
    void *ptr;

    if (size == 0)
    {
        return NULL;
    } else
    {
        ptr = malloc(size);
    }

    if (!ptr)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

// realloc wrapper
void *setMemoryAllocation(void *ptr, size_t size)
{
    void *newPtr;

    newPtr = realloc(ptr, size);

    if (!newPtr && size != 0)
    {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    } else
    {
        return newPtr;
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