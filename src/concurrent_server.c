/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Modulo principal de un servidor web concurrente (pool de threads).
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
#include "http_processing.h"
#include "threadPool.h"
#include "daemon.h"

int sock;
cfg_t *cfg;
threadPool* pool;

/*Seniales parseo de server conf*/
static char* server_root = NULL;
static long int max_clients = 0;
static long int buf_size = 0;
static long int timeout = 0;
static char* listen_port = NULL;
static char* server_signature = NULL;


/********
* FUNCIÓN: void SIGINT_handler()
* DESCRIPCIÓN: Se encarga de manejar la señal SIGINT (Ctrl+C)
********/

void SIGINT_handler(){

    /*Liberamos recursos y salimos del proceso*/

    free(server_root);
    free(server_signature);
    free(listen_port);
    close(sock);
    pool_free(pool);
    syslog(LOG_ERR, "Servidor cerrado: SIGINT");
	exit(-1);
}

/********
* FUNCIÓN:  int handle_petition(int socket, char* inBuffer)
* ARGS_IN:  int socket - descriptor del socket al cual debemos enviar la respuesta
            char* inBuffer - peticion leida
* DESCRIPCIÓN: Maneja una peticion que llegue al servidor
* ARGS_OUT: int - devuelve el retorno de parse_petition (-1 en caso de error,  0 en caso contrario)
********/

int handle_petition(int socket, char* inBuffer){

    return parse_petition(socket, inBuffer, server_signature, server_root, buf_size, timeout);
}

/********
* FUNCIÓN:  int main()
* DESCRIPCIÓN: Funcion main del servidor
* ARGS_OUT: int - 1 en caso de error,  0 en caso contrario
********/

int main(){

	sigset_t set;
	struct addrinfo* addr;

    /*Inicializamos la estructura que necesitaremos en el parseo de server.conf*/

    cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("server_root", &server_root),
		CFG_SIMPLE_INT("max_clients", &max_clients),
		CFG_SIMPLE_STR("listen_port", &listen_port),
		CFG_SIMPLE_STR("server_signature", &server_signature),
		CFG_SIMPLE_INT("buf_size", &buf_size),
        CFG_SIMPLE_INT("timeout", &timeout),
		CFG_END()
	};
	
    cfg = cfg_init(opts, 0);

    /*Parseamos server.conf*/

    if(cfg_parse(cfg, "server.conf") == CFG_PARSE_ERROR){
        syslog(LOG_ERR, "Error en Concurrent Server: Error en cfg_parse()");
        return -1;
    }

    cfg_free(cfg);

    /*Demonizamos el proceso*/

    if(demonizar() < 0){
        syslog(LOG_ERR, "Error en Concurrent Server: Error en demonizar()");
        return -1;
    }
	
	/*La estructura hints se pasa a getaddrinfo con una serie de parametros que queremos que cumpla la direccion que se devuelve en addr*/

	struct addrinfo hints;

	hints.ai_family = AF_INET; // Vamos a manejar direcciones IPv4
	hints.ai_socktype = SOCK_STREAM; // Conexiones TCP
    hints.ai_flags = AI_ALL; 
	hints.ai_protocol = 0; //El 0 elige el protocolo que mejor se adapta al resto de campos introducidos.
	

	/*Definimos una estructura que nos permite caracterizar el socket*/

	if(getaddrinfo(NULL, listen_port, &hints, &addr)){
        free(server_root);
        free(server_signature);
        free(listen_port);
        syslog(LOG_ERR, "Error en Concurrent Server: Error en get_addr_info()");
		return -1;
	}

	/*Iniciamos el socket. Inluye socket(), bind() y listen()*/

    sock = server_socket_setup(addr, max_clients);
	if(sock < 0){
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        syslog(LOG_ERR, "Error en Concurrent Server: Error en server_socket_setup()");
        return -1;
	}

	freeaddrinfo(addr);

    /*Inicializamos el pool de threads, pasando toda la informacion necesaria*/

    if((pool = pool_ini(max_clients, sock, buf_size, handle_petition)) == NULL){
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        close(sock);
    	syslog(LOG_ERR, "Error en Concurrent Server: Error iniciando pool");
    	return -1;
    }


    /*Desbloquea sigint en el hilo principal, nos aseguramos de que sea el unico que maneja la señal*/

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0){
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        close(sock);
        pool_free(pool);
        syslog(LOG_ERR, "Error en Concurrent Server: Error mascara de bloqueo de thread principal");
        return -1;
    }

    /* Asociamos la señal de interrupcion a la funcion que se encarga de manejarla*/

    if(signal (SIGINT, SIGINT_handler) == SIG_ERR){
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        close(sock);
        pool_free(pool);
        syslog(LOG_ERR, "Error en Concurrent Server: Error definiendo SIGINT_handler");
        return -1;
    }

    /*Dejamos al proceso corriendo hasta que reciba la señal de interrupcion*/

	while(1); 
    
	return 0; // No deberia llegar nunca
}