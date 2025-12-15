#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "builtin.h"
#include "utils.h"


extern char **g_environment;

// Exécution d'un builtin avec redirections
static int exec_builtin(command_t *cmd) 
{
    if (!cmd || !cmd->argv[0]) 
    {
        return -1;
    }
   
    int saved_stdout = -1;
    int saved_stdin = -1;
    int ret_status = -1;

    for (int i = 0; builtins[i].name; ++i) 
    {
        if (strcmp(cmd->argv[0], builtins[i].name) == 0) 
        {
            
            // SAUVEGARDE DES DESCRIPTEURS ORIGINAUX SI UNE REDIRECTION EST PRÉVUE
            if (cmd->redir_out || cmd->redir_append) 
            {
                saved_stdout = dup(STDOUT_FILENO);
            }

            if (cmd->redir_in || cmd->heredoc) 
            {
                saved_stdin = dup(STDIN_FILENO);
            }

            if (cmd->redir_out && cmd->outfile) 
            {
                buildtin_redirect_output(cmd->outfile, 0);
            }

            if (cmd->redir_append && cmd->outfile) 
            {
                buildtin_redirect_output(cmd->outfile, 1);
            }

            if (cmd->redir_in && cmd->infile) 
            {
                builtin_redirect_input(cmd->infile);
            }

            if (cmd->heredoc && cmd->heredoc_content) 
            {
                builtin_heredoc_input(cmd->heredoc_content);
            }
   
            // EXÉCUTION
            ret_status = builtins[i].function(cmd->argv);

            // RESTAURATION DES DESCRIPTEURS APRÈS L'EXÉCUTION
            if (saved_stdout != -1) 
            {
                fflush(stdout); 
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }

            if (saved_stdin != -1) 
            {
                dup2(saved_stdin, STDIN_FILENO); 
                close(saved_stdin);
            }
            
            return ret_status;
        }
    }

    return -1; 
}

// Exécution d'une commande système
static int exec_simple(command_t *cmd) 
{
    pid_t pid = fork();

    if (pid == 0) 
    {
        if (cmd->redir_out) 
        {
            buildtin_redirect_output(cmd->outfile, 0);
        }
            
        if (cmd->redir_append) 
        {
            buildtin_redirect_output(cmd->outfile, 1);
        }
            
        if (cmd->redir_in) 
        {
            builtin_redirect_input(cmd->infile);
        }
            
        if (cmd->heredoc) 
        {
             builtin_heredoc_input(cmd->heredoc_content);
        }
           
        execvp(cmd->argv[0], cmd->argv);
        perror("exec");

        exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}

// Exécution générale
int execute(command_t *cmd) 
{
    if (!cmd) 
    {
        return -1;
    }

    // Vérifier si builtin
    int status = exec_builtin(cmd);
    
    if (status != -1) 
    {
        return status;
    }
        
    if (cmd->background) 
    {
        pid_t pid = fork();

        if (pid == 0) 
        {
            exec_simple(cmd);
            exit(0);
        }

        printf("[Process %d lancé en arrière-plan]\n", pid);

        return 0;
    }

    return exec_simple(cmd);
}

// Exécute une seule commande, gérant les redirections, les builtins et les processus externes
int execute_commands(char **arguments, bool is_background, int input_fd, int output_fd) 
{
    char *infile = NULL;
    char *outfile = NULL;
    int append = 0; 
    char **new_args = NULL;
    int new_arg_pos = 0;
    
    if (!arguments || !arguments[0]) 
    {
        return EXIT_SUCCESS;
    }

    // On enlève les symboles de redirection (> >> <)pour ne garder que les "vrais arguments"
    int arg_count = 0;

    while(arguments[arg_count]) 
    {
        arg_count++;
    }
    
    new_args = malloc((arg_count + 1) * sizeof(char*));

    if (!new_args) 
    { 
        perror("malloc new_args failed"); 
        return EXIT_FAILURE; 
    }

    for (int i = 0; arguments[i]; i++) 
    {
        if (strcmp(arguments[i], ">") == 0 || strcmp(arguments[i], ">>") == 0 || strcmp(arguments[i], "<") == 0) 
        {
            if (arguments[i+1])
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
                    infile = arguments[i+1]; 
                } 
                else 
                { 
                    outfile = arguments[i+1]; 
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

    // Vérification builtin 
    int builtin_index = -1;

    for (int i = 0; builtins[i].name; i++) 
    { 
        if (!strcmp(builtins[i].name, new_args[0])) 
        { 
            builtin_index = i; 
            break; 
        } 
    }
    
    // Exécution directe si builtin sans pipe
    if (builtin_index != -1 && !is_piped) 
    {
        int result = builtins[builtin_index].function(new_args); 
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
        
        if (is_background) 
        {
            if (setpgid(0, 0) == -1) 
            { 
                exit(EXIT_FAILURE); 
            }
            
            if (input_fd == STDIN_FILENO && !infile) 
            {
                int dev_null = open("/dev/null", O_RDONLY);

                if (dev_null != -1) 
                {
                    dup2(dev_null, STDIN_FILENO); 
                    close(dev_null);
                }
            }
        }
        
        
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

            dup2(fd_in, STDIN_FILENO); close(fd_in);
        }

        // Sortie (Pipe)
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
            char *command_path = find_command_path(new_args[0]);
            if (command_path == NULL) 
            {
                fprintf(stderr, "Terminus: %s: command not found\n", new_args[0]);
                free(new_args);
                exit(127);
            }

            execve(command_path, new_args, g_environment);
            perror(command_path);
            free(command_path);
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
            
            if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) 
            {
                perror("fail"); 
            }
            
            return 0; 
        } 
        else 
        {
            if (tcsetpgrp(STDIN_FILENO, childPID) == -1) 
            {
                perror("fail"); 
            }
            
            int waitStatus;
            waitpid(childPID, &waitStatus, 0); 

            if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) 
            {
                perror("fail");
            }
            
            return WIFEXITED(waitStatus) ? WEXITSTATUS(waitStatus) : (WIFSIGNALED(waitStatus) ? 128 + WTERMSIG(waitStatus) : EXIT_FAILURE);
        }
    }
}


// Exécute une séquence de commandes connectées par des pipes
int execute_pipeline(char **piped_commands, bool pipeline_is_background) 
{
    int num_commands = 0;

    while(piped_commands[num_commands]) 
    {
        num_commands++;
    }

    int input_fd = STDIN_FILENO; 
    int status = 0;
    
    if (num_commands == 1) 
    {
        char *cmd_copy = strdup(piped_commands[0]);
        bool is_background = pipeline_is_background;
        
        char **arguments = split_input(cmd_copy, &is_background); 

        if (arguments) 
        {
            status = execute_commands(arguments, is_background, STDIN_FILENO, STDOUT_FILENO);
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
        
        bool is_bg_current = false;
        
        if (i < num_commands - 1) 
        { 
            if (pipe(fd) == -1) 
            {

                perror("pipe failed");
                if (input_fd != STDIN_FILENO) 
                { 
                    close(input_fd); 
                }

                return EXIT_FAILURE;
            }
            output_fd = fd[1]; 
        } 
        else 
        {
            output_fd = STDOUT_FILENO; 
            is_bg_current = pipeline_is_background; 
        }

        char *cmd_copy = strdup(cmd_raw);
        
        char **args = split_input(cmd_copy, &is_bg_current); 
        
        if (!args || !args[0]) 
        {
            if (args) 
            { 
                free(args); 
            }

            free(cmd_copy);

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

        int result = execute_commands(args, is_bg_current, input_fd, output_fd);


        if (input_fd != STDIN_FILENO) 
        { 
            close(input_fd); 
        }

        if (i < num_commands - 1)
        {
            close(fd[1]);      
            input_fd = fd[0]; 
        } 
        else 
        {
            if (!is_bg_current) 
            {
                status = result; 
            }
        }
        
        free(args);
        free(cmd_copy);
    }
        
    return status; 
}