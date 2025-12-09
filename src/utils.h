#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void getCurrentWorkingDirectory(char *buffer, size_t size);

void *createMemoryAllocation(size_t size);

void *setMemoryAllocation(void *ptr, size_t size);

void printError(char *message);

void displayArguments(int argc, char **argv);

#endif
