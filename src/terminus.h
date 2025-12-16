#ifndef TERMINUS_H
#define TERMINUS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>   // fork()
#include <sys/wait.h> // wait()

#include "typedef.h"

// SECTION - Directives de préprocesseur
#define PROJECT_NAME "Terminus"
#define DELIMITER "\n\t "

#define BATCH_PARAMETER "-c"
#define BATCH_PARAMETER_LONG "--command"
#define HELP_PARAMETER_LONG "--help"

// Couleurs ANSI
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[1;32m"
#define COLOR_WHITE "\x1b[1;37m"


// SECTION - Prototypes
void displayPrompt(void);

#endif