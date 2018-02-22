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

	//Puntero a funcion handler	
	int (*handler_pointer)(char*, char*);

	//Tamaño del buffer de peticiones de entrada/salida
	long int buffSize;

	//Flag para una finalizacion mas o menos correcta de los hilos.
	int stopThreads;
	
} threadPool;



void* thread_behaviour(void* args){    
	//Casting de la estructura de memoria compartida
    threadPool* datos = (threadPool*) args;
    int socket, listeningSocket, buffSize;
    int (*handler)(char*, char*);
 
    

    //Saca lo que necesita para un correcto funcionamiento al principio de su ejecucion.
    //No es necesario volver a acceder a memoria comapartida A POR DATOS.
    if(pthread_mutex_lock(&datos->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error solicitando mutex (datos)");
    	pthread_exit(NULL);
    }
    
    handler = datos->handler_pointer;
    listeningSocket = datos->listeningSocketDescr;
    buffSize = datos->buffSize;
     
    if(pthread_mutex_unlock(&datos->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error liberando mutex (datos)");
    	pthread_exit(NULL);
    }
    
    char inBuffer[buffSize];
    char outBuffer[buffSize];
 
    while(1){


    	if(pthread_mutex_lock(&datos->sharedMutex) != 0){
    		syslog(LOG_ERR, "Error en thread: Error solicitando mutex (condicion de parada)");
    		pthread_exit(NULL);
    	}
    	
    	if(datos->stopThreads){

	    	if(pthread_mutex_unlock(&datos->sharedMutex) != 0){
	    		syslog(LOG_ERR, "Error en thread: Error liberando mutex (finalizacion de ejecucion)");
	    		pthread_exit(NULL);
	    	}

	    	pthread_exit(NULL);
    	}

    	if(pthread_mutex_unlock(&datos->sharedMutex) != 0){
	    	syslog(LOG_ERR, "Error en thread: Error liberando mutex (condicion de parada)");
	    	pthread_exit(NULL);
	    }
        	
        socket = accept_connection(listeningSocket);
		if(socket == -1){
			syslog(LOG_ERR, "Error en thread: Error en accept_connection()");
			pthread_exit(NULL);
		}

		if(my_receive(socket, inBuffer, buffSize) == -1){
			syslog(LOG_ERR, "Error en thread: Error en my_receive()");
			pthread_exit(NULL);
		}
		
		if(handler(inBuffer, outBuffer) == -1){
			syslog(LOG_ERR, "Error en thread: Error en my_handler()");
			pthread_exit(NULL);
		}
		
		if(my_send(socket, outBuffer) == -1){
			syslog(LOG_ERR, "Error en thread: Error en my_send()");
			pthread_exit(NULL);
		}
		close(socket);

		bzero(inBuffer, strlen(inBuffer));
		bzero(outBuffer, strlen(outBuffer));
    }
    pthread_exit(NULL);
    
}

threadPool* pool_ini(int numThr, int listeningSocketDescr, int buffSize, int(*handler_pointer)(char*, char*)){
	int i;
	threadPool* pool;
	sigset_t set;
	
	//Reserva de memoria para la estructura del pool y sus campos
	pool = (threadPool*) malloc (sizeof (threadPool));
	if(!pool){
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
	pool->stopThreads = 0;


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
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0){
        syslog(LOG_ERR, "Error en Pool: Error mascara de bloqueo de seniales en threads");
        free(pool->threadList);
		free(pool);
        return NULL;
    }

    //Creacion (e inicio de ejecucion) de los threads
	for(i = 0; i < numThr; i++){
    	if(pthread_create(&pool->threadList[i], NULL, thread_behaviour, (void*) pool) != 0){
    		syslog(LOG_ERR, "Error en Pool: No se han podido crear todos los threads");
			free(pool->threadList);
			free(pool);
			return NULL;
    	}
    }

	return pool;
}

void pool_free(threadPool* pool){
	int i;
	
	if(pthread_mutex_lock(&pool->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error al liberar pool (Lock condicion de parada)");
    	pthread_exit(NULL);
    }
    	
    pool->stopThreads = 1;

    if(pthread_mutex_unlock(&pool->sharedMutex) != 0){
	    syslog(LOG_ERR, "Error en thread: Error al liberar pool (Unlock condicion de parada)");
	    pthread_exit(NULL);
	}

	sleep(2);

	for(i = 0; i < pool->numThr; i++){
		pthread_cancel(pool->threadList[i]);
	}
	
	free(pool->threadList);
	free(pool);
	return;	
}