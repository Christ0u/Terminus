#define HISTORY_FILE ".terminus_history"


#include "terminus.h"
#include <errno.h>


// getcwd wrapper
void get_current_working_directory(char *buffer, size_t size)
{
    if (NULL == getcwd(buffer, size))
    {
        perror("getcwd");
    }
}

// malloc wrapper
void *create_memory_allocation(size_t size)
{
    void *ptr;

    if (size == 0)
    {
        return NULL;
    } 
    else
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
void *set_memory_allocation(void *ptr, size_t size)
{
    void *newPtr;

    newPtr = realloc(ptr, size);

    if (!newPtr && size != 0)
    {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    } 
    else
    {
        return newPtr;
    }
}

void print_error(char *message)
{
    printf(COLOR_RED "ERR : %s\n" COLOR_RESET, message);
}

// NOTE - Fonctions temporaires
void display_arguments(int argc, char **argv)
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

char *read_heredoc_content(const char *delimiter) 
{
    char *full_content = NULL; 
    size_t total_len = 0;
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    fprintf(stderr, "> "); 

    while ((read = getline(&line, &len, stdin)) != -1) 
    {
        
        if (read > 0 && line[read - 1] == '\n') 
        {
            line[read - 1] = '\0';
            
            if (strcmp(line, delimiter) == 0) 
            {
                free(line);

                return full_content ? full_content : strdup(""); 
            }
            
            line[read - 1] = '\n'; 

        } 
        else if (strcmp(line, delimiter) == 0) 
        {
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

char **split_pipes(char *string)
{
    char **commands;
    int bufferSize = BUFSIZ;
    int position = 0;

    commands = malloc(bufferSize * sizeof(char*));

    if (!commands) 
    {
        perror("malloc splitPipes failed");

        return NULL;
    }

    for (char *command = strtok(string, "|"); command; command = strtok(NULL, "|"))
    {
        commands[position++] = command;

        if (position >= bufferSize)
        {
            bufferSize *= 2;
            char **new_commands = realloc(commands, bufferSize * sizeof(char*));

            if (!new_commands) 
            {
                perror("realloc splitPipes failed");
                free(commands);

                return NULL;
            }

            commands = new_commands;
        }
    }

    commands[position] = NULL;
    
    return commands;
}

char *find_command_path(const char *cmd) 
{
    if (strchr(cmd, '/') != NULL) 
    {
        if (access(cmd, X_OK) == 0) 
        {
            return strdup(cmd);
        }

        return NULL;
    }

    char *path_env = getenv("PATH");

    if (path_env == NULL) 
    {
        return NULL;
    }

    char *path_copy = strdup(path_env);

    if (path_copy == NULL) {
        return NULL;

    }

    char *dir = strtok(path_copy, ":");
    // Utiliser une taille plus grande si possible, mais 1024 est un minimum raisonnable
    char full_path[1024]; 

    while (dir != NULL) 
    {
        // Construction du chemin complet: dir/cmd
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

        // access(path, X_OK) vérifie l'existence et l'exécutabilité
        if (access(full_path, X_OK) == 0) 
        {
            free(path_copy);

            return strdup(full_path);
        }
        
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    
    return NULL;
}

// Découpe une ligne de commande contenant des "&&" en plusieurs commandes indépendantes.
char **split_and_commands(char *line)
{
    
    char **commands = NULL;
    int bufferSize = 8;
    int position = 0;
    
    
    char *temp_line = strdup(line);
    if (!temp_line) return NULL;
    
    char *search = temp_line;
    while ((search = strstr(search, "&&")) != NULL) 
    {
        *search = '\x01';
        *(search + 1) = ' '; 
        search += 2;
    }
    
    commands = (char **)malloc(bufferSize * sizeof(char*));

    if (!commands) 
    { 
        free(temp_line); 
        return NULL; 
    }

    char *token = strtok(temp_line, "\x01"); 
    
    while (token != NULL) 
    {
        char *start = token;
        while (isspace((unsigned char)*start)) 
        {
            start++;
        }
        
        char *end = start + strlen(start) - 1;

        while (end >= start && isspace((unsigned char)*end)) 
        {
            end--;
        }
        *(end + 1) = '\0';
        
        if (*start != '\0') 
        {
            commands[position++] = strdup(start);

            if (position >= bufferSize) 
            {
                bufferSize *= 2;
                commands = (char **)realloc(commands, bufferSize * sizeof(char*));

                if (!commands) 
                { 
                    free(temp_line); 
                    return NULL; 
                }
            }
        }
        token = strtok(NULL, "\x01");
    }

    free(temp_line);
    commands[position] = NULL;
    return commands;
}

// Lit une ligne entrée par l'utilisateur et utilise getline pour gérer les tailles variables
char *get_user_input(void)
{
  char *buffer = NULL;
  size_t bufferSize;

  if (getline(&buffer, &bufferSize, stdin) == -1)
  {
    free(buffer);
    buffer = NULL;

    if (feof(stdin))
    {
      print_error("EndOfFile");
    } 
    else
    {
      print_error("getline failed");
    }
  }

  return buffer;
}


// Sépare la ligne d'entrée en tokens (arguments), en gérant les guillemets et l'opérateur '&'
char **split_input(char *string, bool *is_background)
{
    size_t bufferSize = 64; 
    char **tokens = (char **)malloc(bufferSize * sizeof(char*));

    if (tokens == NULL) 
    { 
        perror("malloc failed for tokens array");

        return NULL;
    }
    
    unsigned int position = 0;
    *is_background = false;

    char *current_char = string;
    
    // On saute les espaces au début
    while (isspace((unsigned char)*current_char)) 
    {
        current_char++;
    }
    
    char *token_start = current_char;
    bool in_single_quotes = false;
    bool in_double_quotes = false;

    while (*current_char != '\0') 
    {
        // Gestion des guillemets
        if (*current_char == '"' && !in_single_quotes) 
        {
            in_double_quotes = !in_double_quotes;
        } 
        else if (*current_char == '\'' && !in_double_quotes) 
        {
            in_single_quotes = !in_single_quotes;
        } 

        // Si on trouve un espace hors guillemets
        if (isspace((unsigned char)*current_char) && !in_double_quotes && !in_single_quotes) 
        {
            
            if (current_char > token_start) 
            {
                
                size_t token_len = current_char - token_start;
                
                tokens[position] = strndup(token_start, token_len);

                if (tokens[position] == NULL) 
                {
                    perror("strndup failed");

                    return NULL;
                }

                position++;
            }
            
            if (position >= bufferSize) 
            {
                bufferSize *= 2;
                char **temp = realloc(tokens, bufferSize * sizeof(char*));

                if (temp == NULL) 
                {
                    perror("realloc failed");
                    return NULL;
                }

                tokens = temp;
            }

            while (isspace((unsigned char)*current_char)) 
            {
                current_char++;
            }

            token_start = current_char; 

            continue;
        }
        
        current_char++;
    }

    // Dernier token
    if (current_char > token_start) 
    {
        const char *token_end_ptr = current_char;

        while (token_end_ptr > token_start && isspace((unsigned char)*(token_end_ptr - 1))) 
        {
            token_end_ptr--;
        }
        
        size_t token_len = token_end_ptr - token_start;

        if (token_len > 0)
        {
            if (token_len == 1 && *token_start == '&') 
            {
                *is_background = true; 
            } 
            else 
            {
                tokens[position] = strndup(token_start, token_len);

                if (tokens[position] == NULL) 

                {
                    perror("strndup failed");

                    return NULL;
                }

                position++;
            }
        }
    }
    
    // Erreur si guillemets non fermés
    if (in_single_quotes || in_double_quotes) 
    {
        fprintf(stderr, "Terminus: Erreur de syntaxe: guillemets non fermés.\n");

        if (tokens) 
        {
            for (unsigned int i = 0; i < position; i++) 
            {
                free(tokens[i]);
            }

            free(tokens);
        }

        return NULL;
    }
    
    tokens[position] = NULL;

    return tokens;
}

// Construit le chemin complet du fichier d'historique
char *get_history_path() 
{
    const char *home_dir = getenv("HOME");
    
    if (!home_dir) 
    {
        fprintf(stderr, "Avertissement: Variable HOME non définie, utilisant le répertoire courant pour l'historique.\n");
        
        return strdup(HISTORY_FILE); 
    }

    size_t len = strlen(home_dir) + 1 + strlen(HISTORY_FILE) + 1;
    char *path = malloc(len);
    
    if (!path) 
    {
        perror("malloc history path failed");

        return NULL;
    }

    snprintf(path, len, "%s/%s", home_dir, HISTORY_FILE);
    
    return path;
}


// Affiche le contenu du fichier d'historique
void display_history() 
{
    char *path = get_history_path();

    if (!path) {
        return;
    }

    FILE *file = fopen(path, "r");

    if (!file) 
    {
        fprintf(stderr, "Terminus: Aucun historique disponible.\n");
        free(path);

        return;
    }

    char *line = NULL;
    size_t len = 0;
    int command_number = 1;
    
    while (getline(&line, &len, file) != -1) 
    {
        printf(" %4d  %s", command_number++, line);
    }

    free(line);
    fclose(file);
    free(path);
}


// Enregistre une commande dans le fichier d'historique
void save_command_to_history(char *command)
{
    if (!command || command[0] == '\0') 
    {
        return;
    }

    size_t len = strlen(command);

    if (len > 0 && command[len - 1] == '\n') 
    {
        command[len - 1] = '\0';
    }
    
    if (command[0] == '\0') 
    {
        return; 
    }

    char *path = get_history_path();
    
    if (!path) 
    {
        return;
    }

    FILE *file = fopen(path, "a"); 

    if (!file) 
    {
        perror("Could not open history file for writing");
        free(path);

        return;
    }
    
    fprintf(file, "%s\n", command);
    fclose(file);
    free(path);
}

// Remplace un heredoc (<< DELIMITER) par une redirection classique vers un fichier temporaire (< temp_filename)
char *replace_heredoc_arg(char *raw_command, const char *delimiter, const char *temp_filename) 
{
    char search_pattern[256];
    snprintf(search_pattern, sizeof(search_pattern), "<<%s", delimiter);
    char *start_replace = strstr(raw_command, search_pattern);

    if (start_replace == NULL) 
    {
        snprintf(search_pattern, sizeof(search_pattern), "<< %s", delimiter);
        start_replace = strstr(raw_command, search_pattern);
        
        if (start_replace == NULL) 
        {
            fprintf(stderr, "Terminus: Erreur interne, impossible de retrouver le marqueur Heredoc.\n");
            
            return NULL;
        }
    }
    
    size_t len_to_remove = (start_replace - raw_command) + strlen(search_pattern);
    
    len_to_remove = strlen(search_pattern);
    
    
    char insertion_pattern[256 + 3];
    snprintf(insertion_pattern, sizeof(insertion_pattern), "< %s", temp_filename);
    
    size_t len_insert = strlen(insertion_pattern);

    size_t offset_start = start_replace - raw_command; 
    
    char *end_replace = start_replace + len_to_remove;
    
    size_t new_len = offset_start + len_insert + strlen(end_replace) + 1;
    
    char *new_command = (char *)malloc(new_len);

    if (!new_command) 
    {
        perror("malloc failed for new_command (heredoc)");

        return NULL;
    }

    strncpy(new_command, raw_command, offset_start);
    
    strcpy(new_command + offset_start, insertion_pattern);
    
    strcpy(new_command + offset_start + len_insert, end_replace);
    
    return new_command; 
}

// Handler SIGCHLD pour éviter les zombies
void sigchld_handler(int sig)
{
    int saved_errno = errno;
    pid_t pid;
    int status;
    char *cwd = NULL; 

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) 
    {
        fprintf(stderr, "\n[%s: Process %d terminé]\n", PROJECT_NAME, pid);
                
        cwd = getcwd(NULL, 0);

        if (cwd) 
        {
            fprintf(stderr, "%s[ %s ]%s : %s%s%s > ", COLOR_GREEN, PROJECT_NAME, COLOR_RESET, COLOR_WHITE,cwd,COLOR_RESET);
            
            fflush(stderr);
            
            free(cwd);
        }
    }
    
    errno = saved_errno;
}