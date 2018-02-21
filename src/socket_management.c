/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Unidad de manejo de sockets
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include "socket_management.h"



//Estas son las estructuras con la que vamos a trabajar. Pertenece a <netinet/in.h>

/*struct addrinfo {
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    struct  sockaddr* ai_addr;
    char*   ai_canonname;     // canonical name 
    struct  addrinfo* ai_next; // this struct can form a linked list 
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
}; */

int socket_create(struct addrinfo* addr){
    if(addr == NULL)
        return -1;
    return socket(addr->ai_family, addr->ai_socktype,  addr->ai_protocol);
}

int socket_bind(struct addrinfo* addr, int sockval){
    if(addr == NULL || sockval < 0)
        return -1;
    return bind(sockval, addr->ai_addr, addr->ai_addrlen);
}

int socket_listen(int sockval, int max_clients){
    if(max_clients < 0 || sockval < 0)
        return -1;
    return listen (sockval, max_clients);
}

int socket_connect(int clientsock, struct addrinfo* addr){
    if(clientsock < 0 || addr == NULL)
        return -1;
    return connect(clientsock, addr->ai_addr, addr->ai_addrlen);
}

int my_receive (int clientsock, char* inBuffer, long int buf_size){
    if(inBuffer == NULL)
        return -1;
    
    return recv(clientsock, inBuffer, buf_size, 0);
}

int my_send (int clientsock, char* outBuffer){
    if(outBuffer == NULL)
        return -1;

    return send(clientsock, outBuffer, strlen(outBuffer), 0);
}


//Funcion de las diapos algo modificada
int server_socket_setup(struct addrinfo* addr, int max_clients){
	int sockval;
	int opt;
	//Ojo, sockaddr_in para que sea un servidor de internet, sockaddr_un, para que sea local

	//struct sockaddr_in Direccion;

	syslog (LOG_INFO, "Creating socket");
	sockval = socket_create(addr);
	if (sockval < 0 ){
		syslog(LOG_ERR, "Error creating socket");
		return -1;
	}

	//Estas dos asignaciones las satisface el campo ai_addr de addr fijo 
	//Direccion.sin_family=AF_INET; /* TCP/IP family */
	//Direccion.sin_port=htons(NFC_SERVER_PORT); /* Assigning port ¿Por que este puerto? Diria que es al azar*/
	
	//Pa mi que esto es importante
	//Estas no lo tengo nada claro
	//Direccion.sin_addr.s_addr=htonl(INADDR_ANY); /* Accept all adresses */
	//bzero((void *)&(Direccion.sin_zero), 8); 

	//Copypaste de Internet, aun no se que hace, lo dejo porsiaca
	opt = 1;
	if (setsockopt(sockval, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(int)) < 0) {
      syslog(LOG_ERR, "[bindListener:43:setsockopt]");
      return -1;
    }

	syslog (LOG_INFO, "Binding socket");

	if (socket_bind(addr, sockval) < 0){
		syslog(LOG_ERR, "Error binding socket");
		return -1;
	}

	syslog(LOG_INFO, "Listening connections");
	//Listen activa una rutina de recepcion y encolado de peticiones en "segundo plano". Una vez se llama a esta 
	//funcion el socket se queda escuchando (imagino que hasta su cierre o hasta que se le indique que pare)
	if (socket_listen(sockval, max_clients) < 0){
		syslog(LOG_ERR, "Error listenining");
		return -1;
	}

	return sockval;

}


//Como su propio nombre indica, acepta una conexion. Funcion de las diapositivas
int accept_connection(int sockval){
	int desc;
    socklen_t len;
	struct sockaddr Conexion;

	len = sizeof(Conexion);
    desc = accept(sockval, &Conexion, &len);
	if (desc <0){
		syslog(LOG_ERR, "Error accepting connection");
		exit(EXIT_FAILURE);
	}

	return desc;
	
}


int client_socket_setup(struct addrinfo* addr){
	int clientsock;
    
    clientsock = socket_create(addr);
	if(clientsock < 0){
			syslog(LOG_ERR, "Error en clientsock");
			return -1;
	}

	if(socket_connect(clientsock, addr) < 0){
		close(clientsock);
		syslog(LOG_ERR, "Error conectando el cliente");
		return -1;
	}

	return clientsock;
}
