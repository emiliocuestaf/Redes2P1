/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Unidad de manejo de sockets
********************************************************/

#ifndef SOCKET_MANAGEMENT_H
#define SOCKET_MANAGEMENT_H

#include <netdb.h>


int accept_connection(int sockval);
int server_socket_setup(struct addrinfo* addr, int max_clients);
int client_socket_setup(struct addrinfo* addr);

#endif
