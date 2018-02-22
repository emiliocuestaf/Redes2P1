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
	
	//Este mutex regula el acceso de los hilos a memoria compartida, que es justo esta estructura.
	pthread_mutex_t sharedMutex;

	//Hemos visto que accept es thread-safe, por eso no se incluye un semaforo/mutex para su acceso

	int numThr;
	pthread_t* threadList;
	
	int listeningSocketDescr;
	
	int (*handler_pointer)(char*, char*);
	long int buffSize;
	
} threadPool;



void* thread_behaviour(void* args){    
    threadPool* datos = (threadPool*) args;
    int socket, listeningSocket, buffSize;
    int (*handler)(char*, char*);
    char inBuffer[BUFFER_SIZE];
    char outBuffer[BUFFER_SIZE];
    

    if(pthread_mutex_lock(&datos->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error solicitando mutex");
    	pthread_exit(NULL);
    }
    
    handler = datos->handler_pointer;
    listeningSocket = datos->listeningSocketDescr;
    buffSize = datos->buffSize;
     
     if(pthread_mutex_unlock(&datos->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error liberando mutex");
    	pthread_exit(NULL);
    }
    
    while(1){
        	socket = accept_connection(listeningSocket);
			if(socket == -1)
				pthread_exit(NULL);
			my_receive(socket, inBuffer, buffSize);
			handler(inBuffer, outBuffer);
			my_send(socket, outBuffer);
			close(socket);
    }
    
    pthread_exit(NULL);
    
}

threadPool* pool_ini(int numThr, int listeningSocketDescr, int buffSize, int(*handler_pointer)(char*, char*)){
	int i;
	threadPool* pool;
	sigset_t set;
	
	//Reserva de memoria para la estructura del pool y sus campos
	pool = (threadPool*) malloc (sizeof (threadPool));
	if(!pool->threadList){
		syslog(LOG_ERR, "Error en Pool: Error reservando memoria para la estructura pool");
		return NULL;
	}
	pool->numThr = numThr;
	pool->listeningSocketDescr = listeningSocketDescr;
	pool->handler_pointer = handler_pointer;
	pool->buffSize = buffSize;
	pool->threadList = (pthread_t *)malloc(numThr*sizeof(pthread_t));
	if(!pool->threadList){
		syslog(LOG_ERR, "Error en Pool: Error reservando memoria para lista de threads");
		free(pool);
		return NULL;
	}


	//Inicializacion del mutex que regula el uso de las zonas compartidas
	if(pthread_mutex_init(&pool->sharedMutex, NULL) != 0){
		syslog(LOG_ERR, "Error en Pool: Error inicializando MUTEX");
		free(pool->threadList);
		free(pool);
		return NULL;
	}

	//Con esta mascara evitamos que los threads "no principales" no se vean afectados por SIGINT
	//Visto de otra manera, el unico thread que maneja SIGINT es el principal.
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) == 0){
        syslog(LOG_ERR, "Error en Pool: Error mascara de bloqueo de seniales en threads");
        free(pool->threadList);
		free(pool);
        return NULL;
    }

    //Creacion (e inicio de ejecucion) de los threads
	for(i = 0; i < numThr; i++){
    	if(pthread_create(&pool->threadList[i], NULL, thread_behaviour, (void*) pool)!= 0){
    		syslog(LOG_ERR, "Error en Pool: No se han podido crear todos los threads");
			free(pool->threadList);
			free(pool);
			return NULL;
    	}
    }

    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL) == 0){
        syslog(LOG_ERR, "Error en Pool: Error mascara de desbloqueo de seniales en thread principal");
        free(pool->threadList);
		free(pool);
        return NULL;
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