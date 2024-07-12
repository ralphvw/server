#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a structure to hold key-value pairs
typedef struct
{
    char key[50];
    char value[100];
} EnvPair;

// Function to read .env file and parse key-value pairs
int read_env_file(const char *filename, EnvPair *env_pairs, int max_pairs);

#endif // UTILS_H
