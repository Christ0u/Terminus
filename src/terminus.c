#define _POSIX_C_SOURCE 200809L
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
#include <signal.h>   
#include <ctype.h>


// Tableau des commandes internes (builtins), chaque commande est associée à une fonction C
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

// Code de retour de la dernière commande exécutée qui sert notamment pour les opérateurs &&
int exitStatus = 0;

// Liste globale des alias
alias_t *g_aliases = NULL;

// Environnement global utilisé par execve.
extern int exitStatus;

// Affiche le prompt du shell
void display_prompt(void)
{
    printf(COLOR_GREEN "[ %s ]" COLOR_RESET " : " COLOR_WHITE "%s" COLOR_RESET " > ", PROJECT_NAME, getcwd(NULL, 0));
}


// Boucle REPL
int main(int argc, char **argv)
{
    char *userInput = NULL;
    char *input_copy = NULL;
    char **and_commands = NULL;     
    char *heredoc_delimiter = NULL;
    char *heredoc_content = NULL;
    char temp_filename[256] = {0}; 
    char *expanded_cmd = NULL; 
    
    init_environment(); 
    
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, 0) == -1) 
    {
        perror("sigaction failed");
    }

    if (setpgid(0, 0) == -1) 
    {
        perror("setpgid initial failed");
        return EXIT_FAILURE;
    }

    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) 
    {
        perror("tcsetpgrp initial failed");
    }

    while (true)
    {
        heredoc_delimiter = NULL;
        heredoc_content = NULL;
        temp_filename[0] = 0; 
        expanded_cmd = NULL;
        and_commands = NULL;

        display_prompt();
        userInput = get_user_input();

        if (!userInput) 
        {
          printf("\n"); 
          break;   
        }

        if (strspn(userInput, " \t\n") == strlen(userInput)) {
            free(userInput);
            continue;
        }

        save_command_to_history(userInput);
        input_copy = strdup(userInput); 

        if (!input_copy) 
        { 
            perror("strdup failed"); 
            goto cleanup_main_loop;
        }
        
        and_commands = split_and_commands(input_copy); 

        if (!and_commands || !and_commands[0] || and_commands[0][0] == '\0')
        {
          goto cleanup_main_loop;
        }

        for (int i = 0; and_commands[i] != NULL; i++)
        {
            if (i > 0 && exitStatus != 0) 
            {
                break; 
            }

            char *cmd_raw = and_commands[i];
                        
            char *first_word = get_first_word(cmd_raw);
            expanded_cmd = NULL;

            if (first_word) 
            {
                char *alias_value = get_alias_value(first_word);
                if (alias_value != NULL) 
                {
                    expanded_cmd = expand_alias(cmd_raw, alias_value);

                    if (expanded_cmd) {
                        cmd_raw = expanded_cmd; 
                    }
                }
                free(first_word);
            }
            
            char *cmd_to_check = cmd_raw;
            char *first_cmd_copy = strdup(cmd_to_check); 
            bool dummy_bg = false;
            char **first_args = split_input(first_cmd_copy, &dummy_bg);
            
            heredoc_delimiter = NULL;

            if (first_args) 
            {
                for (int j = 0; first_args[j]; j++) 
                {
                    if (strcmp(first_args[j], "<<") == 0 && first_args[j+1]) 
                    {
                        heredoc_delimiter = first_args[j+1];
                        break;
                    }
                }
                free(first_args); 
            }

            free(first_cmd_copy); 

            if (heredoc_delimiter) 
            {
                heredoc_content = read_heredoc_content(heredoc_delimiter);
                if (!heredoc_content) { goto cleanup_iteration; }
                
                strcpy(temp_filename, "/tmp/terminus_heredoc_XXXXXX");
                int fd_temp = mkstemp(temp_filename); 

                if (fd_temp == -1) 
                { 
                    perror("mkstemp failed");
                    goto cleanup_iteration; 
                }

                write(fd_temp, heredoc_content, strlen(heredoc_content));
                close(fd_temp);
                
                char *new_cmd_0 = replace_heredoc_arg(cmd_raw, heredoc_delimiter, temp_filename);

                if (!new_cmd_0) 
                { 
                    goto cleanup_iteration; 
                }
                
                if (expanded_cmd) 
                {
                    free(expanded_cmd);

                }
                cmd_raw = new_cmd_0; 
                expanded_cmd = new_cmd_0; 
            }

            
            char **piped_commands = split_pipes(cmd_raw); 
            
            if (piped_commands && piped_commands[0]) 
            {
                exitStatus = execute_pipeline(piped_commands, false); 
            } 
            else 
            {
                exitStatus = 0; 
            }
            
            if (piped_commands) 
            {
                free(piped_commands);
            }

            cleanup_iteration:; 
            if (expanded_cmd && cmd_raw == expanded_cmd) 
            {
                free(expanded_cmd); 
            }
        }

        
        cleanup_main_loop:

        if (temp_filename[0] != 0) 
        {
            unlink(temp_filename); 
        }
        if (heredoc_content) 
        {
            free(heredoc_content);
        }
        
        if (and_commands) 
        {
            for (int i = 0; and_commands[i] != NULL; i++) 
            {
                free(and_commands[i]);
            }
            free(and_commands);
        }
        
        free(userInput);
        if (input_copy) free(input_copy);

    }
    
    return EXIT_SUCCESS;
}