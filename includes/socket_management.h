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
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>

/********
* FUNCIÓN: int socket_create(struct addrinfo* addr)
* ARGS_IN: struct addrinfo* addr - estructura que caracteriza al socket que queremos crear
* DESCRIPCIÓN: Crea un socket
* ARGS_OUT: int - devuelve el retorno de socket (-1 en caso de error, el descriptor de fichero en caso contrario)
********/

int socket_create(struct addrinfo* addr);

/********
* FUNCIÓN: 	int socket_bind(struct addrinfo* addr, int sockval)
* ARGS_IN: 	struct addrinfo* addr - estructura que contiene informacion con la que asociar el socket
*			int sockval - descriptor del socket
* DESCRIPCIÓN: Asociamos un socket con un puerto contenido en addr
* ARGS_OUT: int - devuelve el retorno de bind (-1 en caso de error, 0 en caso contrario)
********/

int socket_bind(struct addrinfo* addr, int sockval);

/********
* FUNCIÓN: 	int socket_listen(int sockval, int max_clients)
* ARGS_IN: 	int max_clients - numero maximo de conexiones permitidas
*			int sockval - descriptor del socket
* DESCRIPCIÓN: Espera conexiones entrantes del socket
* ARGS_OUT: int - devuelve el retorno de listen (-1 en caso de error, 0 en caso contrario)
********/

int socket_listen(int sockval, int max_clients);

/********
* FUNCIÓN: 	int socket_connect(int clientsock, struct addrinfo* addr)
* ARGS_IN: 	struct addrinfo* addr - estructura que contiene informacion del socket al cual haces la peticion
*			int sockval - descriptor del socket
* DESCRIPCIÓN: Crea una peticion de conexion del socket clientsock a la direccion addr
* ARGS_OUT: int - devuelve el retorno de connect (-1 en caso de error, 0 en caso contrario)
********/

int socket_connect(int clientsock, struct addrinfo* addr);

/********
* FUNCIÓN: 	int my_receive (int clientsock, char* inBuffer, long int buf_size)
* ARGS_IN: 	int clientsock - socket que da la informacion tenemos que leer
			char* inBuffer - buffer donde guardamos lo leido
			long int buf_size - tamanio maximo que podemos leer
* DESCRIPCIÓN: Lee los datos enviados por clientsock y los guarda en inBuffer
* ARGS_OUT: int - devuelve el retorno de recv (-1 en caso de error, bytes leidos en caso contrario)
********/

int my_receive (int clientsock, char* inBuffer, long int buf_size);

/********
* FUNCIÓN: 	int my_send (int clientsock, char* ouBuffer, int length)
* ARGS_IN: 	int clientsock - socket donde vamos a enviar la informacion
			char* outBuffer - buffer donde esta almacenada la informacion
			int length - tamanio de la informacion a enviar
* DESCRIPCIÓN: Envia los datos almacenados en outBuffer al clientsock
* ARGS_OUT: int - devuelve el retorno de send (-1 en caso de error, bytes enviados en caso contrario)
********/

int my_send (int clientsock, char* outBuffer, int length);

/********
* FUNCIÓN: 	int server_socket_setup(struct addrinfo* addr, int max_clients)
* ARGS_IN: 	struct addrinfo* addr - estructura que almacena la informacion que queremos asociar al socket
			int max_clients - numero maximo de conexiones permitidas
* DESCRIPCIÓN: Pone en marcha el socket del servidor
* ARGS_OUT: int - devuelve -1 en caso de error, el descriptor del socket en caso contrario
********/

int server_socket_setup(struct addrinfo* addr, int max_clients);

/********
* FUNCIÓN: 	int accept_connection(int sockval)
* ARGS_IN: 	int sockval - descriptor del socket
* DESCRIPCIÓN: Acepta una conexion de la cola de conexiones del socket
* ARGS_OUT: int - devuelve -1 en caso de error, el descriptor del socket que hemos aceptado en caso contrario
********/

int accept_connection(int sockval);

#endif
