#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_management.h"

/*Primer argumento, una cadena de caracteres a mandar*/
int main(int argc, char** argv ){
	int port;
	int clientsock;
	const char* hostName = "localhost"; //No se muy bien como va esto, creo que seria el dominio
	const char* listenPort = 8080;
	char* buffer[BUFFER_SIZE];
	struct addrinfo* addr;


	if(argc != 2){
		perror("Error en los argumentos de entrada");
		return -1;
	}

	fprintf(stdout, "La frase introducida tiene %d caracteres\n", strlen(argv[1]));

	struct addrinfo hints;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_protocol = 0;

	//Define una estructura que nos permite caracterizar el socket
	if(getaddrinfo(hostName, listenPort, &hints, &addr)){
		perror("Error en getaddrinfo");
		return -1;
	}

	//Como es logico, ademas de tener info del socket del servidor, tenemos que abrir uno aqui
	//Configuramos este socket con las mismas opciones que el del servidor para que sean compatibles
	if(clientsock = client_socket_setup(addr) < 0 ){
		perror("Error socket de cliente");
		return -1;
	}

	freeaddrinfo(addr);
	
	send(clientsock, argv[1], strlen(argv[1]), 0);

	recv(clientfd, buffer, BUFFER_SIZE, 0);

	printf(stdout, "%s", buf);

	close(clientsock);
  	return 0;
}
