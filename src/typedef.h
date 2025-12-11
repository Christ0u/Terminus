typedef struct builtin
{
    const char *name;
    int (*function)(char **);
} builtin;

typedef struct {
    char **argv;

    int redir_out;
    int redir_append;
    int redir_in;
    int heredoc;

    char *outfile; //  >
    char *infile; //  <
    char *heredoc_content; //  <<
    char *heredoc_delimiter; // pour stocker EOF

    int background; // pour &
} command_t;