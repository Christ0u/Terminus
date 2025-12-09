#include "terminus.h"

builtin builtins[] = {
    {.name = "cd", .function = builtin_cd},
    {.name = "pwd", .function = builtin_pwd},
    {.name = "exit", .function = builtin_exit},
    {.name = "echo", .function = builtin_echo},
    {.name = NULL},
};

// Code de retour des commmandes exécutées (builtins et systèmes)
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
    }
    else
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

  // Séparation des jetons en fonction du séparateur défini
  for (char *token = strtok(string, DELIMITER); token; token = strtok(NULL, DELIMITER))
  {
    tokens[position++] = token;

    // Gestion des chaînes de caractères trop grandes
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
int main()
{
  char *userInput = NULL;
  char *input_copy = NULL;
  command_t *cmd = NULL;

  while (true)
  {
    // [1] - Récupération de la saisie
    displayPrompt();

    userInput = getUserInput();
    //printf("%s\n", userInput);
    
    // pour debug pck relou le core dumped là
    if (!userInput) {
      printf("\n");
      break; 
    }

    input_copy = strdup(userInput);
    
    if (input_copy == NULL) {
      perror("strdup failed");
      free(userInput);
      continue;
    }
    
    cmd = parse_line(input_copy); 

    if (cmd == NULL) {
      free(userInput);
      free(input_copy);
      continue;
    }
    
    debug_print_command(cmd);

    // [2] - Exécution
    if (cmd->argv && cmd->argv[0])
    {
      executeBuiltinCommands(cmd->argv);
      // executeSystemCommands(cmd->argv);
    }
    
    // [4] - Libération mémoire
    if (cmd->argv) free(cmd->argv); // Libère le tableau de pointeurs
    free(cmd);                      // Libère la structure command_t
    
    free(userInput);                // Libère la chaîne originale (getline)
    free(input_copy);               // Libère la copie et les jetons (strdup)

    userInput = NULL;
    input_copy = NULL;
    cmd = NULL; 
  }

  return EXIT_SUCCESS;
}