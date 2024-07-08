// handlers.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <libpq-fe.h> // PostgreSQL library
#include "handlers.h"

// Function to handle /users route
void handle_users_route(int sockfd, PGconn *conn)
{
    // Execute SQL query
    const char *query = "SELECT * FROM users";
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        int cols = PQnfields(res);

        // Prepare JSON response
        char json_response[4096]; // Adjust size as needed
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

        // Send HTTP response with JSON data
        send_http_response(sockfd, json_response);
    }
    else
    {
        fprintf(stderr, "Error executing SQL query: %s\n", PQerrorMessage(conn));
        send_http_response(sockfd, "Error retrieving data\n");
    }

    PQclear(res);
}

// Function to handle other routes
void handle_default_route(int sockfd, const char *method, const char *url)
{
    send_http_response(sockfd, "Hello world\n");
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
