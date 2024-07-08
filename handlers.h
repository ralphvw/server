// handlers.h

#ifndef HANDLERS_H
#define HANDLERS_H

#include <libpq-fe.h> // PostgreSQL library

void handle_users_route(int sockfd, PGconn *conn);
void handle_default_route(int sockfd, const char *method, const char *url);
void send_http_response(int sockfd, const char *content);

#endif // HANDLERS_H
