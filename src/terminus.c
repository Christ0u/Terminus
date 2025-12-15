#define HISTORY_FILE ".terminus_history"
#define HISTORY_MAX_SIZE 1000

#include "terminus.h"
#include "parser.h"
#include "builtin.h"
#include "exec.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include "alias.h"
#include <ctype.h>

// Définition du tableau des commandes builtins
builtin_cmd builtins[] = {
    {.name = "cd", .function = builtin_cd},
    {.name = "pwd", .function = builtin_pwd},
    {.name = "exit", .function = builtin_exit},
    {.name = "echo", .function = builtin_echo},
    {.name = "history", .function = builtin_history},
    {.name = "export", .function = builtin_export},
    {.name = "unset", .function = builtin_unset},
    {.name = "alias", .function = builtin_alias},
    {.name = "unalias", .function = builtin_unalias},
    {.name = NULL},
};

int exitStatus = 0;
alias_t *g_aliases = NULL;

void displayPrompt(void)
{
    printf(COLOR_GREEN "[ %s ]" COLOR_RESET " : " COLOR_WHITE "%s" COLOR_RESET " > ", PROJECT_NAME, getcwd(NULL, 0));
}

char *getUserInput(void)
{
    char *buffer = NULL;
    size_t bufferSize;

    if (getline(&buffer, &bufferSize, stdin) == -1)
    {
        free(buffer);
        buffer = NULL;

        if (feof(stdin))
        {
            printError("EndOfFile");
        }
        else
        {
            printError("getline failed");
        }
    }

    return buffer;
}

// Sépare la ligne d'entrée en tokens (arguments), en gérant les guillemets et l'opérateur '&'
char **splitInput(char *string, bool *is_background)
{
    size_t bufferSize = 64;
    char **tokens = (char **)malloc(bufferSize * sizeof(char *));

    if (tokens == NULL)
    {
        perror("malloc failed for tokens array");

        return NULL;
    }

    unsigned int position = 0;
    *is_background = false;

    // Pointeur pour parcourir la chaîne
    char *current_char = string;

    // Ignorer les espaces de tête de la ligne
    while (isspace((unsigned char)*current_char))
    {
        current_char++;
    }

    char *token_start = current_char;
    bool in_single_quotes = false;
    bool in_double_quotes = false;

    while (*current_char != '\0')
    {
        if (*current_char == '"' && !in_single_quotes)
        {
            in_double_quotes = !in_double_quotes;
        }
        else if (*current_char == '\'' && !in_double_quotes)
        {
            in_single_quotes = !in_single_quotes;
        }

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
                char **temp = realloc(tokens, bufferSize * sizeof(char *));

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

// Exécute une seule commande, gérant les redirections, les builtins et les processus externes
int executeCommands(char **arguments, bool is_background, int input_fd, int output_fd)
{
    char *infile = NULL;
    char *outfile = NULL;
    int append = 0;

    char **new_args = NULL;
    int new_arg_pos = 0;

    int original_stdout = -1, original_stdin = -1;

    if (!arguments || !arguments[0])
    {
        return EXIT_SUCCESS;
    }

    int arg_count = 0;
    while (arguments[arg_count])
        arg_count++;

    new_args = malloc((arg_count + 1) * sizeof(char *));

    if (!new_args)
    {
        perror("malloc new_args failed");

        return EXIT_FAILURE;
    }

    for (int i = 0; arguments[i]; i++)
    {
        if (strcmp(arguments[i], ">") == 0 || strcmp(arguments[i], ">>") == 0 || strcmp(arguments[i], "<") == 0)
        {
            if (arguments[i + 1])
            {
                if (strcmp(arguments[i], ">") == 0)
                {
                    append = 0;
                }
                else if (strcmp(arguments[i], ">>") == 0)
                {
                    append = 1;
                }

                if (strcmp(arguments[i], "<") == 0)
                {
                    infile = arguments[i + 1];
                }
                else
                {
                    outfile = arguments[i + 1];
                }

                i++;

                continue;
            }
            else
            {
                fprintf(stderr, "Terminus: Erreur de syntaxe près de '%s': fichier manquant\n", arguments[i]);
                free(new_args);

                return EXIT_FAILURE;
            }
        }
        new_args[new_arg_pos++] = arguments[i];
    }

    new_args[new_arg_pos] = NULL;

    if (!new_args[0])
    {
        free(new_args);
        fprintf(stderr, "Terminus: Erreur de syntaxe (commande manquante).\n");

        return EXIT_FAILURE;
    }

    bool is_piped = (input_fd != STDIN_FILENO || output_fd != STDOUT_FILENO);

    int builtin_index = -1;
    for (int i = 0; builtins[i].name; i++)
    {
        if (!strcmp(builtins[i].name, new_args[0]))
        {
            builtin_index = i;

            break;
        }
    }

    if (builtin_index != -1 && !is_piped)
    {
        int result = EXIT_SUCCESS;
        int fd_out = -1, fd_in = -1;

        if (outfile || infile)
        {
            original_stdout = dup(STDOUT_FILENO);
            original_stdin = dup(STDIN_FILENO);

            if (original_stdout == -1 || original_stdin == -1)
            {
                perror("dup failed");

                result = EXIT_FAILURE;
            }
        }

        if (outfile && result == EXIT_SUCCESS)
        {
            int flags = O_WRONLY | O_CREAT;
            flags |= (append ? O_APPEND : O_TRUNC);
            fd_out = open(outfile, flags, 0644);

            if (fd_out == -1)
            {
                perror(outfile);
                result = EXIT_FAILURE;
            }
            else if (dup2(fd_out, STDOUT_FILENO) == -1)
            {
                perror("dup2 out failed");
                result = EXIT_FAILURE;
            }

            if (fd_out != -1)
            {
                close(fd_out);
            }
        }

        if (infile && result == EXIT_SUCCESS)
        {
            fd_in = open(infile, O_RDONLY);

            if (fd_in == -1)
            {
                perror(infile);
                result = EXIT_FAILURE;
            }
            else if (dup2(fd_in, STDIN_FILENO) == -1)
            {
                perror("dup2 in failed");
                result = EXIT_FAILURE;
            }

            if (fd_in != -1)
            {
                close(fd_in);
            }
        }

        if (result == EXIT_SUCCESS)
        {
            result = builtins[builtin_index].function(new_args);
        }

        if (original_stdout != -1)
        {
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdout);
        }

        if (original_stdin != -1)
        {
            dup2(original_stdin, STDIN_FILENO);
            close(original_stdin);
        }

        free(new_args);

        return result;
    }

    pid_t childPID = fork();

    if (childPID == -1)
    {
        perror("fork failed");
        if (input_fd != STDIN_FILENO)
        {
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO)
        {
            close(output_fd);
        }

        free(new_args);

        return EXIT_FAILURE;
    }
    else if (childPID == 0)
    {

        if (input_fd != STDIN_FILENO)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        else if (infile)
        {
            int fd_in = open(infile, O_RDONLY);

            if (fd_in == -1)
            {
                perror(infile);
                exit(EXIT_FAILURE);
            }

            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (output_fd != STDOUT_FILENO)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        else if (outfile)
        {
            int flags = O_WRONLY | O_CREAT;
            flags |= (append ? O_APPEND : O_TRUNC);

            int fd_out = open(outfile, flags, 0644);

            if (fd_out == -1)
            {
                perror(outfile);
                exit(EXIT_FAILURE);
            }

            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        if (builtin_index != -1)
        {
            exit(builtins[builtin_index].function(new_args));
        }
        else
        {
            execvp(new_args[0], new_args);
            perror(new_args[0]);
            free(new_args);

            exit(EXIT_FAILURE);
        }
    }
    else
    {
        free(new_args);

        if (input_fd != STDIN_FILENO)
        {
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO)
        {
            close(output_fd);
        }

        if (is_background)
        {
            printf("[Process %d lancé en arrière-plan]\n", childPID);

            return childPID;
        }
        else
        {
            int waitStatus;
            waitpid(childPID, &waitStatus, 0);

            return WIFEXITED(waitStatus) ? WEXITSTATUS(waitStatus) : EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

// Exécute une séquence de commandes connectées par des pipes.
int executePipeline(char **piped_commands)
{
    int num_commands = 0;

    while (piped_commands[num_commands])
    {
        num_commands++;
    }

    int input_fd = STDIN_FILENO;

    int status = 0;

    if (num_commands == 1)
    {
        char *cmd_copy = strdup(piped_commands[0]);
        bool is_background = false;

        char **arguments = splitInput(cmd_copy, &is_background);

        if (arguments)
        {
            status = executeCommands(arguments, is_background, STDIN_FILENO, STDOUT_FILENO);
            free(arguments);
        }

        free(cmd_copy);

        return status;
    }

    for (int i = 0; i < num_commands; i++)
    {
        char *cmd_raw = piped_commands[i];
        int fd[2];
        int output_fd = STDOUT_FILENO;

        if (i < num_commands - 1)
        {
            if (pipe(fd) == -1)
            {
                perror("pipe failed");

                return EXIT_FAILURE;
            }

            output_fd = fd[1];
        }
        else
        {
            output_fd = STDOUT_FILENO;
        }

        char *cmd_copy = strdup(cmd_raw);
        bool is_background = (i == num_commands - 1) && (strstr(cmd_raw, "&") != NULL);
        char **args = splitInput(cmd_copy, &is_background);

        if (!args || !args[0])
        {
            if (args)
            {
                free(args);
                free(cmd_copy);
            }

            if (i < num_commands - 1)
            {
                close(fd[0]);
                close(fd[1]);
            }

            if (input_fd != STDIN_FILENO)
            {
                close(input_fd);
            }

            return EXIT_FAILURE;
        }

        int result = executeCommands(args, is_background, input_fd, output_fd);

        if (i < num_commands - 1)
        {
            input_fd = fd[0];
        }
        else
        {
            if (!is_background)
            {
                status = result;
            }
        }

        if (args)
            free(args);

        free(cmd_copy);
    }

    return status;
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

    if (!path)
    {
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

// Remplace le marqueur Heredoc (<<DELIMITER) par une redirection vers un fichier temporaire
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
    // size_t len_raw = strlen(raw_command);

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

// Boucle REPL
int main(int argc, char **argv)
{
    char *userInput = NULL;
    char *input_copy = NULL;
    char **piped_commands = NULL;
    char *heredoc_delimiter = NULL;
    char *heredoc_content = NULL;
    char temp_filename[256] = {0};
    char *expanded_cmd = NULL;

    // Initialisation
    init_environment();

    // Mode interractif
    if (argc == 1)
    {
        while (true)
        {
            heredoc_delimiter = NULL;
            heredoc_content = NULL;
            temp_filename[0] = 0;
            expanded_cmd = NULL;

            displayPrompt();
            userInput = getUserInput();

            if (!userInput)
            {
                printf("\n");

                break;
            }

            save_command_to_history(userInput);

            input_copy = strdup(userInput);

            if (!input_copy)
            {
                perror("strdup failed");
                free(userInput);
                continue;
            }

            piped_commands = splitPipes(input_copy);

            if (!piped_commands || !piped_commands[0] || piped_commands[0][0] == '\0')
            {
                free(userInput);
                free(input_copy);
                free(piped_commands);

                continue;
            }

            char *first_word = get_first_word(piped_commands[0]);

            if (first_word)
            {
                char *alias_value = get_alias_value(first_word);

                if (alias_value != NULL)
                {
                    expanded_cmd = expand_alias(piped_commands[0], alias_value);

                    if (expanded_cmd)
                    {
                        piped_commands[0] = expanded_cmd;
                    }
                    else
                    {
                        fprintf(stderr, "Terminus: Erreur lors de l'expansion d'alias (malloc).\n");
                    }
                }

                free(first_word);
            }

            char *cmd_to_check = piped_commands[0];

            char *first_cmd_copy = strdup(cmd_to_check);
            bool dummy_bg = false;
            char **first_args = splitInput(first_cmd_copy, &dummy_bg);

            if (first_args)
            {
                for (int i = 0; first_args[i]; i++)
                {
                    if (strcmp(first_args[i], "<<") == 0 && first_args[i + 1])
                    {
                        heredoc_delimiter = first_args[i + 1];

                        break;
                    }
                }

                free(first_args);
            }

            free(first_cmd_copy);

            if (heredoc_delimiter)
            {
                heredoc_content = read_heredoc_content(heredoc_delimiter);

                if (!heredoc_content)
                {
                    fprintf(stderr, "Terminus: Erreur lors de la lecture du Here Document.\n");
                    goto cleanup;
                }

                strcpy(temp_filename, "/tmp/terminus_heredoc_XXXXXX");
                int fd_temp = mkstemp(temp_filename);

                if (fd_temp == -1)
                {
                    perror("mkstemp failed");
                    goto cleanup;
                }

                write(fd_temp, heredoc_content, strlen(heredoc_content));
                close(fd_temp);

                char *new_cmd_0 = replace_heredoc_arg(
                    piped_commands[0],
                    heredoc_delimiter,
                    temp_filename);

                if (!new_cmd_0)
                {
                    fprintf(stderr, "Terminus: Erreur lors de l'injection Heredoc.\n");
                    goto cleanup;
                }

                if (expanded_cmd)
                {
                    free(expanded_cmd);
                }

                piped_commands[0] = new_cmd_0;
                expanded_cmd = new_cmd_0;
            }

            executePipeline(piped_commands);

        cleanup:

            if (temp_filename[0] != 0)
            {
                unlink(temp_filename);
            }

            if (heredoc_content)
            {
                free(heredoc_content);
            }

            if (expanded_cmd)
            {
                free(expanded_cmd);
            }

            free(userInput);
            free(input_copy);
            free(piped_commands);

            userInput = NULL;
            input_copy = NULL;
            piped_commands = NULL;
        }
    }
    // Mode batch
    else if (argc >= 2 && strcmp(argv[1], BATCH_DELIMITER) == 0)
    {
        char *batchParameterInput;
        char **batchCommands;
        bool isBackgrounded = false;

        if (argc == 3)
        {
            // Récupération de la valeur de l'argument -c
            batchParameterInput = strdup(argv[2]);

            if (!batchParameterInput)
            {
                perror("strdup failed");

                // Libération mémoire
                free(batchParameterInput);

                return EXIT_FAILURE;
            }

            // Récupération de la commande à exécuter
            batchCommands = splitInput(batchParameterInput, &isBackgrounded);

            // Exécution de la commande en mode batch
            executeCommands(batchCommands, isBackgrounded, STDIN_FILENO, STDOUT_FILENO);

            // Libération mémoire
            free(batchParameterInput);
        }
        else
        {
            printError("Arguments invalides\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        printError("Paramètres invalides\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}