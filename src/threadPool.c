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
	int numThr;
	pthread_t* threadList;
	int listeningSocketDescr;
	int (*handler_pointer)(char*, char*);
	long int buffSize;
	//Hemos visto que accept es thread-safe (nos fiamos 100% de stackoverflow)
	//int acceptSemId;
} threadPool;



void* thread_behaviour(void* args){    
    threadPool* datos = (threadPool*) args;
    int socket;
    char inBuffer[BUFFER_SIZE];
    char outBuffer[BUFFER_SIZE];

    int (*handler)(char*, char*) = datos->handler_pointer;

    while(1){
        	socket = accept_connection(datos->listeningSocketDescr);
			if(socket == -1)
				pthread_exit(NULL);
			my_receive(socket, inBuffer);
			handler(inBuffer, outBuffer);
			my_send(socket, outBuffer);
			close(socket);
    }
    
    pthread_exit(NULL);
    
}

threadPool* pool_ini(int numThr, int listeningSocketDescr, int(*handler_pointer)(char*, char*)){
	int i;
	threadPool* pool;
	sigset_t set;
	
	pool = (threadPool*) malloc (sizeof (threadPool));
	pool->numThr = numThr;
	pool->listeningSocketDescr = listeningSocketDescr;
	pool->handler_pointer = handler_pointer;
	
	pool->threadList = (pthread_t *)malloc(numThr*sizeof(pthread_t));

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) == 0)
        syslog(LOG_ERR, "error mascara de bloqueo de SIGINT de threads");

	for(i = 0; i < numThr; i++){
    	if(pthread_create(&pool->threadList[i], NULL, thread_behaviour, (void*) pool)!= 0){
    		i--;
    	}
    }
	
	return pool;
}

void pool_free(threadPool* pool){
	int i;
	for(i = 0; i < pool->numThr; i++){
		pthread_cancel(pool->threadList[i]);
	}
	free(pool->threadList);
	free(pool);
	return;	
}