#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include "confuse.h"
#include "socket_management.h"

#define BUFFER_SIZE 10000 //No se que poner

int test = 0;

void SIGINT_handler();


//De momento va a contestar a todo con un mensaje fijo y un numero generado por una variable global. (Esto habra que quitarlo luego) 
int handle_petition(int clientsock){
	char inBuffer[BUFFER_SIZE];
	char outBuffer[BUFFER_SIZE];

	if(recv(clientsock, inBuffer, BUFFER_SIZE, 0) < 0){
        perror("Error en recv");
        return -1;
    }
	//Ahora no hacemos nada con lo que nos mandan, en algun momento tendremos aqui un interprete de HTTP y esas cosas

	/*sprintf(outBuffer, "Wohohoho esto es el paso %d de la reconquista de Movistar!!! La cadena enviada tenia %zu caracteres\n\n", test, strlen(inBuffer));*/

    sprintf(outBuffer, "%c", inBuffer[0]);
	test++;

	if(send(clientsock, outBuffer, strlen(outBuffer), 0) < 0){
        perror("Error en send");
        return -1;
    }

    return 0;
}

int main(/*int argc, char **argv*/){

	int sock,i=0;
	//int port;
	int clientsock[1000];
	const char* hostName = "localhost"; //No se muy bien como va esto, creo que seria el dominio
	struct addrinfo* addr;

    static char* server_root = NULL;
    static long int max_clients = 0;
    static char* listen_port = NULL;
    static char* server_signature = NULL;

    if(signal (SIGINT, SIGINT_handler)==SIG_ERR){
        perror("Error definiendo SIGINT_handler");
        return -1;
    }

    cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("server_root", &server_root),
		CFG_SIMPLE_INT("max_clients", &max_clients),
		CFG_SIMPLE_STR("listen_port", &listen_port),
		CFG_SIMPLE_STR("server_signature", &server_signature),
		CFG_END()
	};
	cfg_t *cfg;

    cfg = cfg_init(opts, 0);
    cfg_parse(cfg, "server.conf");

    printf("server_root: %s\n", server_root);
    printf("max_clients: %ld\n", max_clients);
    printf("listen_port: %s\n", listen_port);
    printf("server_signature: %s\n", server_signature);
	
	//ESTA ES UNA IMPLEMENTACION MUY BASICA QUE HABRA 	QUE MEJORAR
	//La estructura hints se pasa a getaddrinfo con una serie de parametros que queremos que cumpla la direccion que se devuelve en addr
	struct addrinfo hints;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // = SOCK_DGRAM si queremos UDP en lugar de TCP
	                  //No obstante, hay algunas funciones mas abajo como accept() o listen() que creo que no fucnionan con udp
    hints.ai_flags = AI_ALL; // not sure why
	hints.ai_protocol = 0;
	//El 0 elige el protocolo que mejor se adapta al resto de campos introducidos.

	//Define una estructura que nos permite caracterizar el socket
	if(getaddrinfo(hostName, listen_port, &hints, &addr)){
		perror("Error en getaddrinfo");
		return -1;
	}

	//Inicia el socket. Inluye socket(), bind() y listen()
    sock = server_socket_setup(addr);
	if(sock < 0){
		perror("Error en server_setup");
	}

	freeaddrinfo(addr);


	while(1){

		//Esta funcion incluye el accept()
        clientsock[i] = accept_connection(sock);
        printf("\n vamo a ver que cliente viene: %d \n", clientsock[i]);;
		if(clientsock[i] < 0){
			perror("Error en accept_connection");
		}
		if(handle_petition(clientsock[i]) < 0){
		    close(clientsock[i]);
            break;
        }
        i++;
		//close(clientsock);
	}
    
    /*Habra que meter estas cosas en el handle de ctrl-c*/

    cfg_free(cfg);
    free(server_root);
    free(server_signature);
    free(listen_port);

	//No se por que pongo esto si no va a salir del bucle
	close(sock);
	return 0;
}


void SIGINT_handler(){

    printf("Aquí habrá que liberar y cerrar cosas");

}

