// handlers.h

#ifndef HANDLERS_H
#define HANDLERS_H

#include <libpq-fe.h> // PostgreSQL library

void handle_users_route(int sockfd, PGconn *conn);
void handle_default_route(int sockfd, const char *method, const char *url);

#endif // HANDLERS_H
