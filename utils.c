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
int read_env_file(const char *filename, EnvPair *env_pairs, int max_pairs)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening .env file");
        return -1;
    }

    char line[150];
    int count = 0;

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = '\0';

        char *delimiter = strchr(line, '=');
        if (delimiter)
        {
            *delimiter = '\0';
            strncpy(env_pairs[count].key, line, sizeof(env_pairs[count].key) - 1);
            strncpy(env_pairs[count].value, delimiter + 1, sizeof(env_pairs[count].value) - 1);
            count++;

            if (count >= max_pairs)
                break;
        }
    }

    fclose(file);
    return count;
}
