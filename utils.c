#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <sys/socket.h>

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

void prepare_json_response(char *json_response, PGresult *res)
{
    int rows = PQntuples(res);
    int cols = PQnfields(res);

    strcpy(json_response, "[");

    for (int i = 0; i < rows; i++)
    {
        strcat(json_response, "{");

        for (int j = 0; j < cols; j++)
        {
            char *value = PQgetvalue(res, i, j);
            char *name = PQfname(res, j);

            strcat(json_response, "\"");
            strcat(json_response, name);
            strcat(json_response, "\":\"");
            strcat(json_response, value);
            strcat(json_response, "\"");

            if (j < cols - 1)
            {
                strcat(json_response, ",");
            }
        }

        strcat(json_response, "}");

        if (i < rows - 1)
        {
            strcat(json_response, ",");
        }
    }

    strcat(json_response, "]");
}

// Function to send HTTP response
void send_http_response(int sockfd, const char *content)
{
    const char *response_fmt = "HTTP/1.0 200 OK\r\n"
                               "Content-Length: %zu\r\n"
                               "Content-Type: application/json\r\n"
                               "\r\n"
                               "%s";

    char response[4096]; // Adjust size as needed
    snprintf(response, sizeof(response), response_fmt, strlen(content), content);

    send(sockfd, response, strlen(response), 0);
}
