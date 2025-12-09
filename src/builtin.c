#include "terminus.h"

int builtin_exit(char **arguments)
{
    //printf("Built-in exit called\n");
    (void)arguments;

    exit(EXIT_SUCCESS);
}