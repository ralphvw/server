#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <libpq-fe.h>
#include "utils.c"
#include "handlers.h"

#define DBNAME "your_database_name"
#define DBUSER "your_username"
#define DBPASS "your_password"
#define DBHOST "localhost" // or your PostgreSQL server hostname
#define DBPORT "5432"

#define MAX_CLIENTS 10

// Example function to demonstrate IPv6 socket programming

void handle_error(const char *msg)
{
    perror(msg);
    exit(1);
}

void send_http_response(int sockfd, const char *content)
{
    const char *response_fmt = "HTTP/1.0 200 OK\r\n"
                               "Content-Length: %zu\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "%s";

    char response[1024]; // Adjust size as needed
    snprintf(response, sizeof(response), response_fmt, strlen(content), content);

    send(sockfd, response, strlen(response), 0);
}

int main()
{
    struct addrinfo hints, *res, *p;
    int sockfd, new_fd, max_fd; // new_fd will hold the newly accepted socket descriptor
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    char client_ip[INET6_ADDRSTRLEN];
    fd_set master_fds, read_fds;
    int client_fds[MAX_CLIENTS];
    int i, max_clients = 0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Use AF_UNSPEC for IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Use AI_PASSIVE for server socket

    // Get address info using getaddrinfo
    int status = getaddrinfo(NULL, "8080", &hints, &res);
    if (status != 0)
    {
        handle_error("getaddrinfo");
    }

    // Loop through all the results and bind to the first valid one
    for (p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            continue;
        }

        // For IPv6, bind using in6addr_any
        if (p->ai_family == AF_INET6)
        {
            struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)p->ai_addr;
            addr_in6->sin6_addr = in6addr_any;
        }

        // Bind the socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        handle_error("bind");
    }

    freeaddrinfo(res); // Free the linked list

    // Listen for incoming connections
    if (listen(sockfd, 10) == -1)
    { // backlog of 10 connections
        handle_error("listen");
    }

    printf("Server is now listening on port 8080...\n");

    // Initialize file descriptor sets for select
    FD_ZERO(&master_fds);
    FD_SET(sockfd, &master_fds);
    max_fd = sockfd;

    EnvPair env_pairs[5]; // Adjust size based on the number of variables in .env
    int num_env_pairs = read_env_file(".env", env_pairs, 5);

    if (num_env_pairs < 0)
    {
        fprintf(stderr, "Failed to read .env file\n");
        exit(1);
    }

    char *dbname = NULL;
    char *dbuser = NULL;
    char *dbpass = NULL;
    char *dbhost = NULL;
    char *dbport = NULL;

    // Find values for database connection parameters
    for (int i = 0; i < num_env_pairs; i++)
    {
        if (strcmp(env_pairs[i].key, "DBNAME") == 0)
            dbname = env_pairs[i].value;
        else if (strcmp(env_pairs[i].key, "DBUSER") == 0)
            dbuser = env_pairs[i].value;
        else if (strcmp(env_pairs[i].key, "DBPASS") == 0)
            dbpass = env_pairs[i].value;
        else if (strcmp(env_pairs[i].key, "DBHOST") == 0)
            dbhost = env_pairs[i].value;
        else if (strcmp(env_pairs[i].key, "DBPORT") == 0)
            dbport = env_pairs[i].value;
    }

    // Check if all required parameters are found
    if (!dbname || !dbuser || !dbpass || !dbhost || !dbport)
    {
        fprintf(stderr, "Missing required database connection parameters in .env file\n");
        exit(1);
    }

    // Initialize PostgreSQL connection
    char conn_str[200]; // Adjust size as needed
    snprintf(conn_str, sizeof(conn_str), "dbname=%s user=%s password=%s host=%s port=%s",
             dbname, dbuser, dbpass, dbhost, dbport);
    PGconn *conn = PQconnectdb(conn_str);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
    }

    printf("Connected to database!\n");

    while (1)
    {                          // Infinite loop for handling multiple connections
        read_fds = master_fds; // Copy master set to select set

        // Use select to monitor sockets
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            handle_error("select");
        }

        // Check for activity on sockets
        for (i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == sockfd)
                { // New incoming connection
                    addr_size = sizeof client_addr;
                    new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
                    if (new_fd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        // Add new connection to master set
                        FD_SET(new_fd, &master_fds);
                        if (new_fd > max_fd)
                        {
                            max_fd = new_fd; // Update max_fd if necessary
                        }

                        // Print client information
                        if (client_addr.ss_family == AF_INET)
                        {
                            struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
                            inet_ntop(AF_INET, &s->sin_addr, client_ip, sizeof client_ip);
                        }
                        else
                        { // AF_INET6
                            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_addr;
                            inet_ntop(AF_INET6, &s->sin6_addr, client_ip, sizeof client_ip);
                        }
                        printf("New connection from %s on socket %d\n", client_ip, new_fd);
                    }
                }
                else
                { // Data from an existing client
                    // Handle data or close the socket
                    char buffer[1024];
                    int nbytes = recv(i, buffer, sizeof(buffer), 0);
                    if (nbytes <= 0)
                    {
                        // Connection closed or error
                        if (nbytes == 0)
                        {
                            printf("Socket %d hung up\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i);               // Close the socket
                        FD_CLR(i, &master_fds); // Remove from master set
                    }
                    else
                    {
                        char method[16]; // Assuming method won't exceed 15 characters
                        char url[1024];  // Assuming URL won't exceed 1023 characters
                        sscanf(buffer, "%s %s", method, url);

                        printf("Method: %s\n", method);
                        printf("URL: %s\n", url);

                        if (strcmp(url, "/users") == 0 && strcmp(method, "GET") == 0)
                        {
                            handle_users_route(i, conn);
                        }

                        // Echo received data back to client (for demonstration)
                        printf("Received %d bytes from socket %d: %s\n", nbytes, i, buffer);
                        send_http_response(i, "Hello world\n"); // Echo back to client
                    }
                }
            }
        }
    }

    // Close the listening socket (never actually reached in this example)
    close(sockfd);

    PQfinish(conn);

    return 0;
}
