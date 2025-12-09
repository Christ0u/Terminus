typedef struct builtin
{
    const char *name;
    int (*function)(char **);
} builtin;

typedef struct command_s {
    char **argv;        

    int has_pipe;         
    int redir_in;         
    int redir_out;        
    int redir_append;     
    int heredoc;          

    int has_and;         
    int has_or;           
    int background;       

    char *filename;
} command_t;