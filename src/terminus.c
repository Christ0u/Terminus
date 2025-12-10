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
  printf("[%s] > ", PROJECT_NAME);
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

int executeCommands(char **arguments)
{
  // SECTION - Builtin commands execution

  int i = 0;
  const char *currentBuiltin;

  while ((currentBuiltin = builtins[i].name))
  {
    if (!strcmp(currentBuiltin, arguments[0]))
    {
      if (builtins[i].function(arguments) == EXIT_FAILURE)
      {
        return EXIT_FAILURE;
      }

      return EXIT_SUCCESS;
    }

    i++;
  }

  // SECTION - Native bash commands execution

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

// Boucle REPL
int main()
{
  char *userInput;
  char **arguments;

  while (true)
  {
    // [1] - Récupération de la saisie
    displayPrompt();

    userInput = getUserInput();
    // printf("%s\n", userInput);

    // Gestion des EndOfFile (CTRL + D)
    if (!userInput)
    {
      break;
    }

    // [2] - Evaluation de la saisie
    arguments = splitInput(userInput);

    // Si aucun token n'est défini (ligne vide ou caractères séparateurs uniquement)
    if (!arguments || !arguments[0])
    {
      free(userInput);
      free(arguments);
      continue;
    }

    // for (int i = 0; arguments[i]; ++i)
    // {
    //   printf("%s\n", arguments[i]);
    // }

    // [3] - Exécution
    executeCommands(arguments);

    // [4] - Libération mémoire
    free(userInput);
    free(arguments);
  }

  return EXIT_SUCCESS;
}