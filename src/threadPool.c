/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Funciones principales de un pool estatico de threads
********************************************************/

#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <pthread.h>
#include "socket_management.h"
#include "threadPool.h"

#define BUFFER_SIZE 1000



typedef struct _threadPool{
	int numProc;
	pthread_t* threadList;
	int listeningSocketDescr;
	void (*handler_pointer)(char*, char*);
	//Hemos visto que accept es thread-safe (nos fiamos 100% de stackoverflow)
	//int acceptSemId;
} threadPool;

void* thread_behaviour(void* n){    
    threadPool* datos = (threadPool*) n;
    int socket;
    char inBuffer[BUFFER_SIZE];
    char outBuffer[BUFFER_SIZE];

    void (*handler)(char*, char*) = datos->handler_pointer;
    while(1){

        	socket = accept_connection(datos->listeningSocketDescr);
			//while(1) (para mantener conexion abierta)
			my_receive(socket, inBuffer);
			handler(inBuffer, outBuffer);
			my_send(socket, outBuffer);
    }
    
    pthread_exit(NULL);
    
}

threadPool* pool_ini(int numProc, int n, int listeningSocketDescr, void(*handler_pointer)(char*, char*)){
	int i;
	threadPool* pool;

	pool = (threadPool*) malloc (sizeof (threadPool));
	pool->numProc = numProc;
	pool->listeningSocketDescr = listeningSocketDescr;
	pool->handler_pointer = handler_pointer;

	//inisemaforos (muy importante iniciar antes)
	
	pool->threadList = (pthread_t *)malloc(n*sizeof(pthread_t));

	for(i = 0; i < pool->numProc; i++){
    	pthread_create(&pool->threadList[i], NULL, thread_behaviour, (void*) pool);
    }

	return pool;
}



void pool_free(threadPool* pool){
	int i;
	for(i = 0; i < pool->numProc; i++){
		pthread_cancel(pool->threadList[i]);
	}
	free(pool->threadList);
	free(pool->handler_pointer);
	free(pool);
	return;	
}