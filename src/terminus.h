#ifndef TERMINUS_H
#define TERMINUS_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>

#include "typedef.h"
#include "builtin.h"
#include "utils.h"
#include "exec.h"

// SECTION - Directives de préprocesseur
#define PROJECT_NAME "Terminus"
#define DELIMITER "\n\t "

#define HISTORY_FILE ".terminus_history"
#define HISTORY_MAX_SIZE 1000

#define BATCH_PARAMETER "-c"
#define BATCH_PARAMETER_LONG "--command"
#define HELP_PARAMETER_LONG "--help"

// Couleurs ANSI
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[1;32m"
#define COLOR_WHITE "\x1b[1;37m"

#endif