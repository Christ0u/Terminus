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

// Boucle REPL
int main(int argc, char **argv)
{
  char *userInput;

  while (true)
  {
    // [1] - Récupération de la saisie (R)

    displayPrompt();

    userInput = getUserInput();
    printf("%s\n", userInput);

    // [2] - Evaluation de la saisie (E)

    // [3] - Exécution
  }

  return EXIT_SUCCESS;
}