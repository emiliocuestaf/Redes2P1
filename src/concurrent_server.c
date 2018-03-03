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

//Seniales parseo de server conf
static char* server_root = NULL;
static long int max_clients = 0;
static long int buf_size = 0;
static char* listen_port = NULL;
static char* server_signature = NULL;


/*Funcion que se encarga de manejar la señal SIGINT (Ctrl+C)*/

void SIGINT_handler(){
	cfg_free(cfg);
    free(server_root);
    free(server_signature);
    free(listen_port);
    close(sock);
    pool_free(pool);
    syslog(LOG_ERR, "Servidor cerrado: SIGINT");
	exit(-1);
};

//De momento va a contestar a todo con un mensaje fijo y un numero generado por una variable global. (Esto habra que quitarlo luego) 
int handle_petition(int socket, char* inBuffer){

    parse_petition(socket, inBuffer, server_signature, server_root, buf_size); // ATENCION, CAMBIAR PRIMER ARGUMENTO
    
    sleep(1);

    return 0;
}

int main(){

	sigset_t set;
	struct addrinfo* addr;

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

    if(cfg_parse(cfg, "./src/server.conf") == CFG_PARSE_ERROR){
        syslog(LOG_ERR, "Error en Concurrent Server: Error en cfg_parse()");
        return -1;
    }


    /*if(demonizar() < 0){
        syslog(LOG_ERR, "Error en Concurrent Server: Error en demonizar()");
        return -1;
    }*/
	
	//La estructura hints se pasa a getaddrinfo con una serie de parametros que queremos que cumpla la direccion que se devuelve en addr
	struct addrinfo hints;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_ALL; 
	hints.ai_protocol = 0;
	//El 0 elige el protocolo que mejor se adapta al resto de campos introducidos.

	//Define una estructura que nos permite caracterizar el socket
	if(getaddrinfo(NULL, listen_port, &hints, &addr)){
		cfg_free(cfg);
        free(server_root);
        free(server_signature);
        free(listen_port);
        syslog(LOG_ERR, "Error en Concurrent Server: Error en get_addr_info()");
		return -1;
	}

	//Inicia el socket. Inluye socket(), bind() y listen()
    sock = server_socket_setup(addr, max_clients);
	if(sock < 0){
        cfg_free(cfg);
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        syslog(LOG_ERR, "Error en Concurrent Server: Error en server_socket_setup()");
        return -1;
	}

	freeaddrinfo(addr);

    if((pool = pool_ini(max_clients, sock, buf_size, handle_petition)) == NULL){
        cfg_free(cfg);
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        close(sock);
    	syslog(LOG_ERR, "Error en Concurrent Server: Error iniciando pool");
    	return -1;
    }


    //Desbloquea sigint en el hilo principal
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0){
        cfg_free(cfg);
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        close(sock);
        pool_free(pool);
        syslog(LOG_ERR, "Error en Concurrent Server: Error mascara de bloqueo de thread principal");
        return -1;
    }

    //Senial de finalizacion
    if(signal (SIGINT, SIGINT_handler) == SIG_ERR){
        cfg_free(cfg);
        free(server_root);
        free(server_signature);
        free(listen_port);
        freeaddrinfo(addr);
        close(sock);
        pool_free(pool);
        syslog(LOG_ERR, "Error en Concurrent Server: Error definiendo SIGINT_handler");
        return -1;
    }

	while(1);
    
	return 0;
}
