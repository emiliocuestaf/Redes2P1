#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_management.h"

int test = 0;


//De momento va a contestar a todo con un mensaje fijo y un numero generado por una variable global. (Esto habra que quitarlo luego) 
int handle_petition(int clientsock){
	char* inBuffer[BUFFER_SIZE];
	char* outBuffer[BUFFER_SIZE];

	recv(handler, inBuffer, BUFFER_SIZE);
	//Ahora no hacemos nada con lo que nos mandan, en algun momento tendremos aqui un interprete de HTTP y esas cosas

	sprintf(outBuffer, "Wohohoho esto es el paso %d de la reconquista de Movistar!!! La cadena enviada tenia %d caracteres\n\n", test, strlen(inBuffer));
	test++;

	send(clientsock, outBuffer, strlen(outBuffer), 0);
}

int main(int argc, char ∗∗argv){

	int sock;
	int port;
	int clientsock;
	const char* hostName = "localhost"; //No se muy bien como va esto, creo que seria el dominio
	const char* listenPort = 8080;
	struct addrinfo* addr;
	
	//ESTA ES UNA IMPLEMENTACION MUY BASICA QUE HABRA 	QUE MEJORAR
	//La estructura hints se pasa a getaddrinfo con una serie de parametros que queremos que cumpla la direccion que se devuelve en addr
	struct addrinfo hints;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; 
	//hints.ai_socktype = SOCK_DGRAM si queremos UDP en lugar de TCP
	//No obstante, hay algunas funciones mas abajo como accept() o listen() que creo que no fucnionan con udp
	hints.ai_protocol = 0;
	//El 0 elige el protocolo que mejor se adapta al resto de campos introducidos.

	//Define una estructura que nos permite caracterizar el socket
	if(getaddrinfo(NULL, listenPort, &hints, &addr)){
		perror("Error en getaddrinfo");
		return -1;
	}

	//Inicia el socket. Inluye socket(), bind() y listen()
	if(sock = server_socket_setup(addr) < 0){
		perror("Error en server_setup");
	}

	freeaddrinfo(addr);


	while(1){

		//Esta funcion incluye el accept()
		if((clientsock = accept_connection(sock)) < 0){
			perror("Error en accept_connection");
		}
		handle_petition(clientsock);
		close(clientsock);
	}

	//No se por que pongo esto si no va a salir del bucle
	close(sock);
	return 0;
}

