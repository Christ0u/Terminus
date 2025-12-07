#include "Terminus.h"

/*
int main(int argc, char **argv)
{
  // displayArguments(argc, argv);

  int status;

  // Création d'un processus fils dédié à l'exécution de la commande passée en argument
  pid_t childPID = fork();

  // Erreur lors de la création du processus fils
  if (childPID == -1)
  {
    perror("fork");
    return 1;
  }
  // Processus fils
  else if (childPID == 0)
  {
    // Exécution de la commande avec les paramètres
    execvp(argv[1], argv + 1);
  }
  // Processus père
  else
  {
    wait(&status);
  }

  return EXIT_SUCCESS;
}

*/

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

// Boucle REPL
int main()
{
  char *userInput;
  char **arguments;

  while (true)
  {
    // [1] - Récupération de la saisie (R)

    displayPrompt();

    userInput = getUserInput();
    // printf("%s\n", userInput);

    // [2] - Evaluation de la saisie (E)
    arguments = splitInput(userInput);
    for (int i = 0; arguments[i]; ++i)
    {
      printf("%s\n", arguments[i]);
    }

    // [3] - Exécution

    // [4] - Libération mémoire
    free(userInput);
    free(arguments);
  }

  return EXIT_SUCCESS;
}