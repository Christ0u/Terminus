#include "terminus.h"
#include "parser.h"
#include "builtin.h"
#include "exec.h"

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
    displayPrompt();

    // Lecture
    userInput = getUserInput();
    if (!userInput) {
      printf("\n");
      break;   
    }

    // Copie pour parser
    input_copy = strdup(userInput);
    if (!input_copy) {
      perror("strdup failed");
      free(userInput);
      continue;
    }

    // Parser
    cmd = parse_line(input_copy);
    if (!cmd) {
      free(userInput);
      free(input_copy);
      continue;
    }

    parse_redirections(cmd);
    debug_print_command(cmd);

    // Exécution unique : builtin OU externe
    execute(cmd);

    // Nettoyage
    if (cmd->argv) {
      free(cmd->argv);
    } 

    if (cmd->outfile) {
      free(cmd->outfile);
    }

    if (cmd->infile) {
      free(cmd->infile);
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