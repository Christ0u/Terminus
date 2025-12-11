#include "terminus.h"
#include "parser.h"
#include "builtin.h"
#include "exec.h"
#include "utils.h"

builtin_cmd builtins[] = {
  {.name = "cd", .function = builtin_cd},
  {.name = "pwd", .function = builtin_pwd},
  {.name = "exit", .function = builtin_exit},
  {.name = "echo", .function = builtin_echo},
  {.name = NULL},
};

int exitStatus = 0;

void displayPrompt(void)
{
  printf("[Terminus] >");
  fflush(stderr);
}

char *getUserInput(void)
{
  char *buffer = NULL;
  size_t bufferSize;

  if (getline(&buffer, &bufferSize, stdin) == -1)
  {
    free(buffer);
    buffer = NULL;

    if (feof(stdin))
    {
      printError("EndOfFile");
    } else
    {
      printError("getline failed");
    }
  }

  return buffer;
}

char **splitInput(char *string)
{
  char **tokens;
  unsigned int position;
  size_t bufferSize;

  bufferSize = BUFSIZ;
  tokens = createMemoryAllocation(bufferSize * sizeof(*tokens));
  position = 0;

  for (char *token = strtok(string, DELIMITER); token; token = strtok(NULL, DELIMITER))
  {
    tokens[position++] = token;

    if (position >= bufferSize)
    {
      bufferSize *= 2;
      tokens = setMemoryAllocation(tokens, bufferSize * sizeof(*tokens));
    }
  }

  tokens[position] = NULL;

  return tokens;
}

void executeBuiltinCommands(char **arguments)
{
  int i = 0;
  const char *currentBuiltin;

  while ((currentBuiltin = builtins[i].name))
  {
    if (!strcmp(currentBuiltin, arguments[0]))
    {
      builtins[i].function(arguments);
      return;
    }

    i++;
  }
}

/*
int executeSystemCommands(char **arguments)
{
  int waitStatus;

  // Création d'un processus fils dédié à l'exécution de la commande passée en argument
  pid_t childPID = fork();

  // Erreur lors de la création du processus fils
  if (childPID == -1)
  {
    perror("fork");
    return EXIT_FAILURE;
  }
  // Processus fils
  else if (childPID == 0)
  {
    // Exécution de la commande avec les arguments
    execvp(arguments[0], arguments);
  }
  // Processus père
  else
  {
    wait(&waitStatus);
  }

  return EXIT_SUCCESS;
}
*/

// Boucle REPL
int main(void)
{
  char *userInput = NULL;
  char *input_copy = NULL;
  command_t *cmd = NULL;

  while (true)
  {
    // [1] - Affichage du prompt et lecture de l'entrée
    displayPrompt();

    userInput = getUserInput();
    if (!userInput) {
      printf("\n");
      break;   
    }

    // [2] - Copie et Parsing
    input_copy = strdup(userInput);
    if (!input_copy) {
      perror("strdup failed");
      free(userInput);
      continue;
    }

    cmd = parse_line(input_copy);
    if (!cmd) {
      free(userInput);
      free(input_copy);
      continue;
    }

    // [3] - Lecture interactive du contenu du Here Document (si << est présent)
    if (cmd->heredoc && cmd->heredoc_delimiter) 
    {
      cmd->heredoc_content = read_heredoc_content(cmd->heredoc_delimiter);
        
      if (!cmd->heredoc_content && cmd->heredoc_delimiter) 
      { 
        fprintf(stderr, "Terminus: Erreur lors de la lecture du Here Document.\n");
      
        free(cmd->heredoc_delimiter);
        free(cmd->argv);
        free(cmd);
        free(userInput);
        free(input_copy);
        continue;
      }
    }

    debug_print_command(cmd);
    
    // [4] - Exécution
    execute(cmd); // La fonction execute gère les redirections

    // [5] - Nettoyage complet
    if (cmd->argv) {
      free(cmd->argv);
    } 

    if (cmd->outfile) {
      free(cmd->outfile);
    }

    if (cmd->infile) {
      free(cmd->infile);
    }
    
    if (cmd->heredoc_delimiter) {
      free(cmd->heredoc_delimiter);
    }

    if (cmd->heredoc_content) {
      free(cmd->heredoc_content);
    }
    
    free(cmd);

    free(userInput);
    free(input_copy);

    userInput = NULL;
    input_copy = NULL;
    cmd = NULL;
  }

  return EXIT_SUCCESS;
}