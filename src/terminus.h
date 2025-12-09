#ifndef TERMINUS_H
#define TERMINUS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>   // fork()
#include <sys/wait.h> // wait()

// SECTION - Directives de préprocesseur
#define PROJECT_NAME "Terminus"
#define DELIMITER "\n\t "

// Couleurs ANSI
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"

// SECTION - Prototypes
void displayPrompt(void);
char *getUserInput(void);
void printError(char *message);
void *createMemoryAllocation(size_t size);
void *setMemoryAllocation(void *ptr, size_t size);

void getCurrentWorkingDirectory(char *buffer, size_t size);
void displayArguments(int argc, char **argv);

int builtin_exit(char **);

#endif