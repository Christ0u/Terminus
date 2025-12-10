#ifndef PARSER_H
#define PARSER_H

#include "terminus.h"

command_t *parse_line(char *line);
void debug_print_command(command_t *cmd);
void parse_redirections(command_t *cmd);

#endif
