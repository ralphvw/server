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
#include "utils.h"

// Function to handle /users route
void handle_users_route(int sockfd, PGconn *conn)
{
    // Execute SQL query
    const char *query = "SELECT * FROM users";
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) == PGRES_TUPLES_OK)
    {
        char json_response[4096]; // Adjust size as needed
        prepare_json_response(json_response, res);

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
