/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Modulo principal de un servidor web.
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <syslog.h>
#include "confuse.h"
#include "socket_management.h"
#include "picohttpparser.h"
#include "http_processing.h"
#include "daemon.h"

int sock;
cfg_t *cfg;

//Seniales parseo de server conf
static char* server_root = NULL;
static long int max_clients = 0;
static long int buf_size = 0;
static char* listen_port = NULL;
static char* server_signature = NULL;

//Estructura para pasar argumentos al thread
typedef struct _args{
    int dim;
} args;

/*Funcion que se encarga de manejar la señal SIGINT (Ctrl+C)*/

void SIGINT_handler(){
	cfg_free(cfg);
    free(server_root);
    free(server_signature);
    free(listen_port);
	close(sock);
	exit(-1);
};

//De momento va a contestar a todo con un mensaje fijo y un numero generado por una variable global. (Esto habra que quitarlo luego) 
int handle_petition(int clientsock, char* inBuffer, char* outBuffer){

    parse_petition(clientsock, inBuffer, outBuffer, server_signature, server_root, buf_size);
    
    sleep(1);

    return 0;
}

int main(/*int argc, char **argv*/){

    //char* inBuffer;
	//char* outBuffer;

	do_daemon();
	
	int clientsock;
	struct addrinfo* addr;
   

    //Senial de finalizacion
    if(signal (SIGINT, SIGINT_handler)==SIG_ERR){
        perror("Error definiendo SIGINT_handler");
        return -1;
    }

    //Parseo de server.conf
    cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("server_root", &server_root),
		CFG_SIMPLE_INT("max_clients", &max_clients),
		CFG_SIMPLE_STR("listen_port", &listen_port),
		CFG_SIMPLE_STR("server_signature", &server_signature),
		CFG_SIMPLE_INT("buf_size", &buf_size),
		CFG_END()
	};
	

    cfg = cfg_init(opts, 0);
    cfg_parse(cfg, "server.conf");

    char inBuffer[buf_size];
    char outBuffer[buf_size];
	//La estructura hints se pasa a getaddrinfo con una serie de parametros que queremos que cumpla la direccion que se devuelve en addr
	struct addrinfo hints;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; 
	// = SOCK_DGRAM si queremos UDP en lugar de TCP
	//No obstante, hay algunas funciones mas abajo como accept() o listen() que creo que no fucnionan con udp
    hints.ai_flags = AI_ALL; // not sure why
	hints.ai_protocol = 0;
	//El 0 elige el protocolo que mejor se adapta al resto de campos introducidos.

	//Define una estructura que nos permite caracterizar el socket
	if(getaddrinfo(NULL, listen_port, &hints, &addr)){
		perror("Error en getaddrinfo");
		return -1;
	}

	//Inicia el socket. Inluye socket(), bind() y listen()
    sock = server_socket_setup(addr, max_clients);
	if(sock < 0){
		perror("Error en server_setup");
	}

	freeaddrinfo(addr);

	//Rutina de procesamiento
	while(1){

		//Esta funcion incluye el accept()
        clientsock = accept_connection(sock);
		if(clientsock < 0){
			perror("Error en accept_connection");
		}
		
	    if(my_receive(clientsock, inBuffer, buf_size) < 0){
            perror("Error en recv");
            return -1;
        }
		//Solo el hijo procesa la peticion (con el sleep) para que el padre pueda seguir aceptando
		if(handle_petition(clientsock, inBuffer, outBuffer) < 0){
			    close(clientsock);
	            break;
	    }
        
	    close(clientsock);
    	//	exit(EXIT_SUCCESS);
    	
		//close(clientsock);
	}
    
	return 0;
}
