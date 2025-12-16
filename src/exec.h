#ifndef EXEC_H
#define EXEC_H

#include "terminus.h"

int execute_commands(char **arguments, bool is_background, int input_fd, int output_fd);
int execute_pipeline(char **piped_commands, bool pipeline_is_background);

#endif