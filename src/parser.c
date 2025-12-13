#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "utils.h"

// ----------------------------------------------------
//   Debug
// ----------------------------------------------------
void debug_print_command(command_t *cmd) 
{
    printf("=== DEBUG COMMAND ===\n");

    for (int i = 0; cmd->argv && cmd->argv[i]; i++) 
    {
        printf("argv[%d] = '%s'\n", i, cmd->argv[i]);
    }

    printf("redir_out      = %d\n", cmd->redir_out);
    printf("redir_append   = %d\n", cmd->redir_append);
    printf("redir_in       = %d\n", cmd->redir_in);
    printf("heredoc        = %d\n", cmd->heredoc);
    printf("outfile        = %s\n", cmd->outfile ? cmd->outfile : "(null)");
    printf("infile         = %s\n", cmd->infile ? cmd->infile : "(null)");
    printf("heredoc_content= %s\n", cmd->heredoc_content ? cmd->heredoc_content : "(null)");
    printf("======================\n");
}
