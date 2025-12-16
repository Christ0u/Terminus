#ifndef ALIAS_H
#define ALIAS_H

#include <stdlib.h>

typedef struct alias_s
{
    char *name;
    char *value;
    struct alias_s *next;
} alias_t;

extern alias_t *g_aliases;

void trim_quotes(char *in_out);
char *get_alias_value(const char *name);
void set_alias(const char *name, const char *value);
void unset_alias(const char *name);
char *get_first_word(const char *cmd_line);
char *expand_alias(char *command_line, char *alias_value);

#endif