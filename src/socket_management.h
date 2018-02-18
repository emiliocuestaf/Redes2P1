#ifndef SOCKET_MANAGEMENT_H
#define SOCKET_MANAGEMENT_H
#include <netdb.h>


int accept_connection(int sockval);
int server_socket_setup(struct addrinfo* addr);
int client_socket_setup(struct addrinfo* addr);

#endif