typedef struct builtin
{
    const char *name;
    int (*function)(char **);
} builtin;