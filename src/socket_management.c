#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <unistd.h>
#include "socket_management.h"


#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 1000


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


//Funcion de las diapos algo modificada
int server_socket_setup(struct addrinfo* addr){
	int sockval;
	int opt;
	//Ojo, sockaddr_in para que sea un servidor de internet, sockaddr_un, para que sea local

	//struct sockaddr_in Direccion;

	syslog (LOG_INFO, "Creating socket");
	if ( (sockval = socket(addr->ai_family, addr->ai_socktype, 0)) < 0 ){
		syslog(LOG_ERR, "Error creating socket");
		exit(EXIT_FAILURE);
	}

	//Estas dos asignaciones las satisface el campo ai_addr de addr fijo 
	//Direccion.sin_family=AF_INET; /* TCP/IP family */
	//Direccion.sin_port=htons(NFC_SERVER_PORT); /* Assigning port ¿Por que este puerto? Diria que es al azar*/
	
	//Estas no lo tengo nada claro
	//Direccion.sin_addr.s_addr=htonl(INADDR_ANY); /* Accept all adresses */
	//bzero((void *)&(Direccion.sin_zero), 8); /*Parece bastante irrelevante poner esto a 0 o no segun Internet*/

	//Copypaste de Internet, aun no se que hace, lo dejo porsiaca
	opt = 1;
	if (setsockopt(sockval, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(int)) < 0) {
      perror("[bindListener:43:setsockopt]");
      return -1;
    }

	syslog (LOG_INFO, "Binding socket");

	if (bind(sockval, addr->ai_addr, addr->ai_addrlen) < 0){
		syslog(LOG_ERR, "Error binding socket");
		exit(EXIT_FAILURE);
	}

	syslog(LOG_INFO, "Listening connections");
	//Listen activa una rutina de recepcion y encolado de peticiones en "segundo plano". Una vez se llama a esta 
	//funcion el socket se queda escuchando (imagino que hasta su cierre o hasta que se le indique que pare)
	if (listen (sockval, MAX_CONNECTIONS)<0){
		syslog(LOG_ERR, "Error listenining");
		exit(EXIT_FAILURE);
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
	
	//launch_service(desc);
	//wait_finished_services();
}


/*void launch_service(int connval){
	int pid;
	long type, aux;

	pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid == 0) return;
	
	syslog (LOG_INFO, "New access");
	recv(connval, &aux, sizeof(long), 0);
	type = ntohl(aux);
	
	database_access(connval, type, NULL);
	close(connval);
	syslog (LOG_INFO, "Exiting service");
	exit(EXIT_SUCCESS);
}*/


int client_socket_setup(struct addrinfo* addr){
	int clientsock;
    
    clientsock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if(clientsock < 0){
			perror("Error en clientsock");
			return -1;
	}


	if(connect(clientsock, addr->ai_addr, addr->ai_addrlen) < 0){
		close(clientsock);
		perror("Error conectando el cliente");
		return -1;
	}

	return clientsock;
}