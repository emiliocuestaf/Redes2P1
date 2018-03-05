/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Funciones principales de un pool estatico de threads
********************************************************/

#include "threadPool.h"

/********
* ESTRUCTURA: threadPool
* DESCRIPCIÓN: Estructura que almacena informacion de un pool estatico de hilos mientras este se esta ejecutando
* CAMPOS:
*			- pthread_mutex_t sharedMutex: Regula el acceso de los hilos a memoria compartida. En este codigo sera justo esta misma estructura.
*			- int numThr: Numero de hilos corriendo en el Pool
*			- int pthread_t* listeningSocketDescr: Socket desde el que se escucha por peticiones.
*			- int (*handler_pointer)(int, char*) : Puntero a la funcion que determina como se procesa la peticion recibida Y ENVIA LA RESPUESTA
*			- long int buffSize: tamaño de los buffer en los que se recibe y se envian las peticiones/respuestas.
*			- int stopThreads: Flag utilizado para avisar a los hilos de que deben finalizar su ejecución.
*
********/
typedef struct _threadPool{
	
	pthread_mutex_t sharedMutex;
	int numThr;
	pthread_t* threadList;
	int listeningSocketDescr;
	int (*handler_pointer)(int, char*);
	long int buffSize;
	int stopThreads;
	
} threadPool;


/********
* FUNCIÓN AUXILIAR:  thread_behaviour(void* args)
* ARGS_IN:  void* args - estructura que contiene los argumentos que se necesitan 
						 para la aceptacion de peticiones y su procesamiento.
						 El casting a void* es necesario por la libreria lpthread,
						 pero en realidad se trata de un threadPool* (estructura presente arriba)             
* DESCRIPCIÓN: Funcion que determina el funcionamiento de cada uno de los hilos del pool.
* ARGS_OUT: No hay retorno. El hilo mantiene la ejecucion hasta que se cierra.
********/
void* thread_behaviour(void* args){    
	//Casting de la estructura de memoria compartida
    threadPool* datos = (threadPool*) args;
    int socket, listeningSocket, buffSize;
    int (*handler)(int, char*);

    

    //Saca lo que necesita para un correcto funcionamiento al principio de su ejecucion.
    //No es necesario volver a acceder a memoria comapartida A POR DATOS.
    if(pthread_mutex_lock(&datos->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error solicitando mutex (datos)");
    	pthread_exit(NULL);
    }
    
    handler = datos->handler_pointer;
    listeningSocket = datos->listeningSocketDescr;
    buffSize = datos->buffSize;
    
    char inBuffer[buffSize];

    memset(inBuffer,0,buffSize);
    if(pthread_mutex_unlock(&datos->sharedMutex) != 0){
    	syslog(LOG_ERR, "Error en thread: Error liberando mutex (datos)");
    	pthread_exit(NULL);
    }
    
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
		
		if(handler(socket, inBuffer) == -1){
			syslog(LOG_ERR, "Error en thread: Peticion mal procesada");
			//No se debe cancelar el hilo si no puede procesar bien una peticion, debe seguir funcionando y procesar otras
		}
		
		close(socket);
		bzero(inBuffer, strlen(inBuffer));
    }
    pthread_exit(NULL);
    
}

/********
* FUNCIÓN:  threadPool* pool_ini(int numThr, int listeningSocketDescr, int buffSize, int(*handler_pointer)(int, char*));
* ARGS_IN:  int numThr - numero de hilos que va a mantener constantemente abiertos. 
            int listeningSocketDescr - socket que esta escuchando por el puerto, a trave de el se aceptan las peticiones.
            int buffSize - tamaño del buffer de las peticiones tanto de entrada como de respuesta.
            int(*handler_pointer)(int, char*) - puntero a la funcion que determinara el manejo de las peticiones recibidas.
* DESCRIPCIÓN: Funcion que sirve para inicializar un thread Pool estatico.
* ARGS_OUT: Un puntero al pool de threads creado o NULL en caso de error.
********/
threadPool* pool_ini(int numThr, int listeningSocketDescr, int buffSize, int(*handler_pointer)(int, char*)){
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

/********
* FUNCIÓN:  void pool_free(threadPool* pool);  
* ARGS_IN:  threadPool* pool - pool de hilos que se va a liberar, cancelando todos los hilos.             
* DESCRIPCIÓN: Funcion que sirve para liberar un thread Pool estatico.
* ARGS_OUT: void
********/
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

	sleep(1);

	for(i = 0; i < pool->numThr; i++){
		pthread_cancel(pool->threadList[i]);
	}
	
	free(pool->threadList);
	free(pool);
	return;	
}