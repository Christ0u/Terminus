#include "alias.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Retire les guillemets doubles entourant une chaîne pour les valeurs d'alias ou export
void trim_quotes(char *in_out)
{
    if (!in_out)
    {
        return;
    }

    size_t len = strlen(in_out);

    if (len >= 2 && in_out[0] == '"' && in_out[len - 1] == '"')
    {
        memmove(in_out, in_out + 1, len - 2);
        in_out[len - 2] = '\0';
    }
}

// Recherche et retourne la valeur (commande) associée à un nom d'alias dans g_aliases
char *get_alias_value(const char *name)
{
    alias_t *current = g_aliases;

    if (name == NULL)
    {
        return NULL;
    }

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current->value;
        }
        current = current->next;
    }

    return NULL;
}

// Ajoute ou met à jour un alias (paire nom/valeur) dans g_aliases
void set_alias(const char *name, const char *value)
{
    alias_t *current = g_aliases;
    alias_t *prev = NULL;
    char *new_value_copy = NULL;

    if (!name || !value)
    {
        return;
    }

    // Alloue une copie indépendante de la nouvelle valeur
    new_value_copy = strdup(value);

    if (new_value_copy == NULL)
    {
        perror("strdup failed for alias value");
        return;
    }

    // Parcourt la liste chaînée tant que 'current' n'est pas NULL
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {

            free(current->value);
            current->value = new_value_copy;

            return;
        }

        prev = current;
        current = current->next;
    }

    // Alloue la mémoire pour la nouvelle structure alias_t
    alias_t *new_alias = (alias_t *)malloc(sizeof(alias_t));

    if (new_alias == NULL)
    {
        perror("malloc failed for new alias");
        free(new_value_copy);

        return;
    }

    // Alloue et copie le nom de l'alias
    new_alias->name = strdup(name);

    if (new_alias->name == NULL)
    {
        perror("strdup failed for alias name");
        free(new_value_copy);
        free(new_alias);

        return;
    }

    new_alias->value = new_value_copy;
    new_alias->next = NULL;

    // Rattachement du nouveau noeud
    if (g_aliases == NULL)
    {
        g_aliases = new_alias;
    }
    else
    {
        prev->next = new_alias;
    }
}

// Supprime un alias de la liste chaînée g_aliases en recherchant le nom correspondant
void unset_alias(const char *name)
{
    alias_t *current = g_aliases;
    alias_t *prev = NULL;

    if (!name)
    {
        return;
    }

    // Parcourt la liste tant que le noeud courant n'est pas NULL
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            if (prev == NULL)
            {
                g_aliases = current->next;
            }
            else
            {
                prev->next = current->next;
            }

            free(current->name);
            free(current->value);
            free(current);

            return;
        }

        prev = current;
        current = current->next;
    }
}

// Extrait le premier mot (token) d'une ligne de commande
char *get_first_word(const char *cmd_line)
{
    if (!cmd_line)
    {
        return NULL;
    }

    while (isspace((unsigned char)*cmd_line))
    {
        cmd_line++;
    }

    const char *start = cmd_line;
    bool in_quotes = false;

    while (*cmd_line != '\0')
    {
        if (*cmd_line == '"')
        {
            in_quotes = !in_quotes;
        }

        if (isspace((unsigned char)*cmd_line) && !in_quotes)
        {
            break;
        }

        cmd_line++;
    }

    size_t len = cmd_line - start;

    if (len == 0)
    {
        return NULL;
    }

    char *word = (char *)malloc(len + 1);

    if (!word)
    {
        perror("malloc failed for first_word");

        return NULL;
    }

    strncpy(word, start, len);
    word[len] = '\0';
    return word;
}

// Effectue l'expansion de l'alias en remplaçant l'alias (premier mot) par sa valeur et en concaténant les arguments restants
char *expand_alias(char *command_line, char *alias_value)
{

    char *cmd_ptr = command_line;
    bool in_quotes = false;

    // Trouver la fin du premier mot et ignorer les espaces
    while (isspace((unsigned char)*cmd_ptr))
    {
        cmd_ptr++;
    }

    while (*cmd_ptr != '\0')
    {
        if (*cmd_ptr == '"')
        {
            in_quotes = !in_quotes;
        }

        if (isspace((unsigned char)*cmd_ptr) && !in_quotes)
        {
            break;
        }

        cmd_ptr++;
    }

    // Isoler les arguments restants
    while (isspace((unsigned char)*cmd_ptr))
    {
        cmd_ptr++;
    }

    char *rest_of_command = cmd_ptr;

    // Allocation
    size_t alias_len = strlen(alias_value);
    size_t rest_len = strlen(rest_of_command);

    size_t new_len = alias_len + (rest_len > 0 ? 1 : 0) + rest_len + 1;
    char *new_cmd = (char *)malloc(new_len);

    if (!new_cmd)
    {
        perror("malloc failed for expanded_cmd");

        return NULL;
    }

    // Construction de la nouvelle chaîne
    strcpy(new_cmd, alias_value);

    if (rest_len > 0)
    {
        strcat(new_cmd, " ");
        strcat(new_cmd, rest_of_command);
    }

    return new_cmd;
}