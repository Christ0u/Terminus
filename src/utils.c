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

// PAS TEMPORAIRE DONC PAS SUPPRIMER
char *read_heredoc_content(const char *delimiter) {
    char *full_content = NULL; 
    size_t total_len = 0;
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    fprintf(stderr, "> "); 
    while ((read = getline(&line, &len, stdin)) != -1) {
        
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
            
            if (strcmp(line, delimiter) == 0) {
               
                free(line);
                return full_content ? full_content : strdup(""); 
            }
            
            line[read - 1] = '\n'; 
        } else if (strcmp(line, delimiter) == 0) {
            free(line);
            return full_content ? full_content : strdup("");
        }

        
        
        size_t current_line_len = read;
        size_t new_total_len = total_len + current_line_len + 1; // +1 pour '\0'

        char *new_content = realloc(full_content, new_total_len);
        if (new_content == NULL) 
        {
            perror("realloc failed");
            free(full_content);
            free(line);
            return NULL;
        }

        full_content = new_content;
        
        memcpy(full_content + total_len, line, current_line_len); 
        full_content[new_total_len - 1] = '\0';
        total_len += current_line_len; 

        fprintf(stderr, "> ");
    }

    free(line); 

    return full_content;
}