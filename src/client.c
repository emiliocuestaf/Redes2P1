/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Cliente para simular el funcionamiento del servidor
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "socket_management.h"

#define BUFFER_SIZE 10000 //No se que poner

/*Primer argumento, una cadena de caracteres a mandar*/
int main(int argc, char** argv ){
	//int port;
	int clientsock, i;
	const char* hostName =  NULL; //No se muy bien como va esto, creo que seria el dominio
	const char* listenPort = "8080";
	char buffer[BUFFER_SIZE];
	struct addrinfo* addr;


	if(argc != 2){
		perror("Error en los argumentos de entrada");
		return -1;
	}

	fprintf(stdout, "La frase introducida tiene %zu caracteres\n", strlen(argv[1]));

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
	int pid;
    for(i=0; i < 10; i++){
        pid = fork();
        if(pid == 0){
        	clientsock = client_socket_setup(addr);  
			if(clientsock < 0 ){
				perror("Error socket de cliente");
				//return -1;
				exit(EXIT_SUCCESS);
			}			
		    send(clientsock, &(argv[1][i]), 1, 0); // send(clientsock, argv[1], strlen(argv[1]), 0);

		    recv(clientsock, buffer, BUFFER_SIZE, 0);

		    fprintf(stdout, "\n %s \n", buffer);

		    return 0;
		}
	   
    }

    for (i = 0; i<10; i++){
    	wait(NULL);
    }

    freeaddrinfo(addr);

	//close(clientsock);
    return 0;
}
