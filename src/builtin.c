#include "terminus.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "alias.h"
#include "builtin.h"

extern char **environ;
char **g_environment = NULL; // Variable globale pour stocker l'environnement du shell

// Initialise l'environnement du shell en copiant le tableau global 'environ' du système.
void init_environment()
{
    int count = 0;
    char **p;

    for (p = environ; *p != NULL; p++)
    {
        count++;
    }

    // Alloue la mémoire pour le tableau de pointeurs g_environment (+1 pour NULL).
    g_environment = (char **)malloc((count + 1) * sizeof(char *));

    if (g_environment == NULL)
    {
        perror("malloc failed for g_environment");

        exit(EXIT_FAILURE);
    }

    // Copie chaque chaîne de l'environnement (environ) vers g_environment.
    for (int i = 0; i < count; i++)
    {
        g_environment[i] = strdup(environ[i]);

        if (g_environment[i] == NULL)
        {
            perror("strdup failed during environment initialization");

            exit(EXIT_FAILURE);
        }
    }

    g_environment[count] = NULL;
}

// Ajoute ou met à jour une variable d'environnement dans le tableau g_environment
int set_env_var(const char *var_assignment)
{
    if (var_assignment == NULL || strchr(var_assignment, '=') == NULL)
    {
        return -1;
    }

    char *clean_assignment = strdup(var_assignment);

    if (!clean_assignment)
    {
        perror("strdup failed");

        return -1;
    }

    char *equal_sign = strchr(clean_assignment, '=');

    char *value_start = equal_sign + 1;

    size_t len = strlen(clean_assignment);

    // Nettoyage des guillemets
    if (*value_start == '"' && clean_assignment[len - 1] == '"')
    {
        clean_assignment[len - 1] = '\0';

        value_start++;

        size_t new_value_len = strlen(value_start);

        size_t name_len = equal_sign - clean_assignment;

        // Reconstruit la chaîne nettoyée ("NAME=value")
        char *temp = (char *)malloc(name_len + 1 + new_value_len + 1);

        if (!temp)
        {
            perror("malloc failed");
            free(clean_assignment);

            return -1;
        }

        strncpy(temp, clean_assignment, name_len);
        temp[name_len] = '=';
        strcpy(temp + name_len + 1, value_start);

        free(clean_assignment);
        clean_assignment = temp;
        equal_sign = strchr(clean_assignment, '=');
    }

    *equal_sign = '\0';
    const char *name = clean_assignment;
    size_t name_len = strlen(name);

    for (int i = 0; g_environment[i] != NULL; i++)
    {
        if (strncmp(g_environment[i], name, name_len) == 0 && g_environment[i][name_len] == '=')
        {
            *equal_sign = '=';
            free(g_environment[i]);
            g_environment[i] = clean_assignment;

            return 0;
        }
    }

    // Ajout d'une nouvelle variable
    *equal_sign = '=';

    int count = 0;

    while (g_environment[count] != NULL)
    {
        count++;
    }

    // Réalloue l'espace pour ajouter la nouvelle variable (+1 pour variable et +1 pour NULL)
    char **new_env = (char **)realloc(g_environment, (count + 2) * sizeof(char *));

    if (new_env == NULL)
    {
        perror("realloc failed for environment");
        free(clean_assignment);

        return -1;
    }

    g_environment = new_env;

    g_environment[count] = clean_assignment;
    g_environment[count + 1] = NULL;

    return 0;
}

// Affiche ou définit des variables d'environnement
int builtin_export(char **args)
{
    if (args[1] == NULL)
    {
        for (char **p = g_environment; *p != NULL; p++)
        {
            printf("%s\n", *p);
        }

        return EXIT_SUCCESS;
    }

    for (int i = 1; args[i] != NULL; i++)
    {
        if (set_env_var(args[i]) != 0)
        {
            fprintf(stderr, "export: format invalide: %s\n", args[i]);

            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

// Supprime une variable d'environnement
int builtin_unset(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "unset: argument manquant\n");

        return EXIT_FAILURE;
    }

    for (int k = 1; args[k] != NULL; k++)
    {
        const char *name_to_unset = args[k];
        int i;

        for (i = 0; g_environment[i] != NULL; i++)
        {
            size_t name_len = strlen(name_to_unset);

            if (strncmp(g_environment[i], name_to_unset, name_len) == 0 && g_environment[i][name_len] == '=')
            {
                free(g_environment[i]);

                int j;

                for (j = i; g_environment[j] != NULL; j++)
                {
                    g_environment[j] = g_environment[j + 1];
                }

                break;
            }
        }
    }

    return EXIT_SUCCESS;
}

// Termine le shell
int builtin_exit(char **arguments)
{
    (void)arguments;

    exit(EXIT_SUCCESS);
}

// Affiche le répertoire de travail courant
int builtin_pwd(char **arguments)
{
    (void)arguments;

    char *currentWorkingDirecetory;
    currentWorkingDirecetory = getcwd(NULL, 0);

    if (currentWorkingDirecetory == NULL)
    {
        perror("getcwd failed");

        return EXIT_FAILURE;
    }
    else
    {
        printf("%s\n", currentWorkingDirecetory);
    }

    return EXIT_SUCCESS;
}

// Affiche les arguments sur la sortie standard
int builtin_echo(char **arguments)
{
    // Si aucun argument ou un argument vide est précisé
    if (!arguments || !arguments[1])
    {
        printf("\n");
        return EXIT_SUCCESS;
    }

    for (int i = 1; arguments[i] != NULL; ++i)
    {
        printf("%s", arguments[i]);

        if (arguments[i + 1])
        {
            printf(" ");
        }
    }

    printf("\n");

    return EXIT_SUCCESS;
}

// Change le répertoire de travail courant
int builtin_cd(char **arguments)
{
    char *path;

    // Si aucun argument ou un argument vide est précisé
    if (!arguments || !arguments[1])
    {
        // Récupération du répertoire HOME de l'utilisateur
        path = getenv("HOME");
    }
    else
    {
        path = arguments[1];
    }

    if (chdir(path) == -1)
    {
        perror("cd failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Gère la redirection de sortie (> ou >>)
int buildtin_redirect_output(const char *filename, int append)
{
    int fd;

    if (append)
    {
        fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    else
    {
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0)
    {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

// Gère la redirection d'entrée (<)
int builtin_redirect_input(const char *filename)
{
    int fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        perror("open");

        return -1;
    }

    if (dup2(fd, STDIN_FILENO) < 0)
    {
        perror("dup2");
        close(fd);

        return -1;
    }

    close(fd);

    return 0;
}

// Gère l'entrée via Here Document (<<) en utilisant un pipe
int builtin_heredoc_input(const char *content)
{
    int fd[2];

    if (pipe(fd) < 0)
    {
        perror("pipe");
        return -1;
    }

    write(fd[1], content, strlen(content));
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);

    return 0;
}

// Affiche l'historique des commandes
int builtin_history(char **args)
{
    (void)args;
    display_history();

    return EXIT_SUCCESS;
}

// Affiche la liste ou crée/met à jour un alias
int builtin_alias(char **args)
{
    int exit_status = EXIT_SUCCESS;

    if (args[1] == NULL)
    {
        alias_t *current = g_aliases;

        while (current != NULL)
        {
            printf("alias %s='%s'\n", current->name, current->value);
            current = current->next;
        }

        return EXIT_SUCCESS;
    }

    for (int i = 1; args[i] != NULL; i++)
    {
        char *equal_sign = strchr(args[i], '=');

        if (equal_sign != NULL)
        {

            char *arg_copy = strdup(args[i]);
            if (!arg_copy)
            {
                perror("strdup failed");
                exit_status = EXIT_FAILURE;

                continue;
            }

            char *eq = strchr(arg_copy, '=');

            *eq = '\0';
            char *name = arg_copy;
            char *value = eq + 1;

            trim_quotes(value);

            set_alias(name, value);

            free(arg_copy);
        }
        else
        {
            char *value = get_alias_value(args[i]);

            if (value)
            {
                printf("alias %s='%s'\n", args[i], value);
            }
            else
            {
                fprintf(stderr, "alias: %s introuvable\n", args[i]);
                exit_status = EXIT_FAILURE;
            }
        }
    }

    return exit_status;
}

// Supprime un ou plusieurs alias de la liste
int builtin_unalias(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "unalias: argument manquant\n");

        return EXIT_FAILURE;
    }

    int exit_status = EXIT_SUCCESS;

    for (int i = 1; args[i] != NULL; i++)
    {
        unset_alias(args[i]);
    }

    return exit_status;
}