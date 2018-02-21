/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Funciones principales de un pool de threads
********************************************************/

#include "procpool.h"

#define BUFFER_SIZE 1000
#define FILEKEY "/bin/cat" /*Util para ftok */
#define KEY1 1300
#define KEY2 3100
#define KEY3 2345

typedef struct _processPool{
	int maxProc;
	int minProc;
	int* staticProcPIDList;
	int listeningSocketDescr;
	void(*handler_pointer)(char*, char*);
	int acceptSemId;
	int maxProcSemId;
	int extraProcNeededId;
} processPool;

int new_dynamic_process(int listeningSocket, void(*handler_pointer)(char*, char*),  int acceptSemId, int extraProxNeededId, int maxProcSemId);
int new_static_process(int listeningSocket, void(*handler_pointer)(char*, char*), int acceptSemId, int extraProcneededId);


processPool* pool_ini(int minProc, int maxProc, int n, int listeningSocketDescr, void(*handler_pointer)(char*, char*)){
	int i;
	int aux;
	int pid;
	ushort ini;
	key_t key;

	processPool pool;
	pool.minProc = minProc;
	pool.maxProc = maxProc;
	pool.listeningSocketDescr = listeningSocketDescr;
	pool.handler_pointer = handler_pointer;

	//inisemaforos (muy importante iniciar antes)
	key = ftok(FILEKEY, KEY1);
	if(Crear_Semaforo(key, 1, &pool.acceptSemId) == ERROR){
		syslog(LOG_ERR, "Error creando semaforo 1 pool procesos");
		return NULL;
	}
	ini = 1;
	if(Inicializar_Semaforo(pool.acceptSemId, &ini) == ERROR){
		syslog(LOG_ERR, "Error inicializando semaforo 1 pool procesos");
		return NULL;
	}

	key = ftok(FILEKEY, KEY2);
	if(Crear_Semaforo(key, 1, &pool.maxProcSemId) == ERROR){
		syslog(LOG_ERR, "Error creando semaforo 2 pool procesos");
		return NULL;
	}
	ini = pool.maxProc;
	if(Inicializar_Semaforo(pool.maxProcSemId, &ini) == ERROR){
		syslog(LOG_ERR, "Error inicializando semaforo 2 pool procesos");
		return NULL;
	}

	key = ftok(FILEKEY, KEY3);
	if(Crear_Semaforo(key, 1, &pool.extraProcNeededId) == ERROR){
		syslog(LOG_ERR, "Error creando semaforo 3 pool procesos");
		return NULL;
	}
	ini = 0;
	if(Inicializar_Semaforo(pool.extraProcNeededId, &ini) == ERROR){
		syslog(LOG_ERR, "Error inicializando semaforo 3 pool procesos");
		return NULL;
	}

	pool.staticProcPIDList = (int *)malloc(n*sizeof(int));

	pid = fork();
	if(pid < 0){
		syslog(LOG_ERR, "Error creando proceso principal del pool");
	}
	if (pid == 0){
		for(i = 0; i < n; i++){
			aux = new_static_process(listeningSocketDescr, handler_pointer, pool.acceptSemId, pool.extraProcNeededId);
			if(aux == -1)
				return NULL;
			pool.staticProcPIDList[i] = aux;
		}
		while(1){
		/*  down(maxThr)
			down(extraNeeded)
			pid = fork()
			if (pid == 0)
				new_dynamic_process(listeningSocketDescr, handler_pointer, pool.acceptSemId, pool.extraProcNeededId, pool.maxProcSemId) */
		}
	}
	
	return &pool;
}

//Devuelve el PID
int new_static_process(int listeningSocket, void(*handler_pointer)(char*, char*), int acceptSemId, int extraProcneededId){
	int pid;
	int socket;
	struct addrinfo* addr;
    char inBuffer[BUFFER_SIZE];
	char outBuffer[BUFFER_SIZE];
	pid = fork();
	if (pid == -1){
		return -1;
	}
	else if (pid == 0){
		while(1){
			
			socket = accept_connection(listeningSocket);
			//mutex off
			//semup more_proccess_needed
			myReceive(addr, inBuffer);
			handler_pointer(inBuffer, outBuffer);
			mySend(addr, outBuffer);
		}
	}
	else{
		return pid;
	}
}

//Lo que diferencia  a un proceso estatico de uno dinamico es qu elos procesos estaticos no finalizarn. Ademas,
//son ellos los que se encargan de permitir que se creen los dinamicos.
int new_dynamic_process(int listeningSocket, void(*handler_pointer)(char*, char*),  int acceptSemId, int extraProxNeededId, int maxProcSemId){
	int pid;
	int socket;
	struct addrinfo* addr;
    char inBuffer[BUFFER_SIZE];
	char outBuffer[BUFFER_SIZE];
	pid = fork();
	if (pid == -1){
		return -1;
	}
	else if (pid == 0){
			//mutex on
			socket = accept_connection(listeningSocket);
			//mutex off
			//semup more_proccess_needed ??
			myReceive(addr, inBuffer);
			handler_pointer(inBuffer, outBuffer);
			mySend(addr, outBuffer);
			//semup(maxprocs)
	}
	else{
		return pid;
	}
}



void pool_free(processPool* pool){
	int i;
	for(i = 0; i < pool->minProc; i++){
		kill(pool->staticProcPIDList[i], SIGINT);
	}
	free(pool->staticProcPIDList);
	free(pool->handler_pointer);

	if(Borrar_Semaforo(pool->acceptSemId) == ERROR){
		syslog(LOG_ERR, "Error eliminando semaforo 1 pool procesos");
	}

	if(Borrar_Semaforo(pool->maxProcSemId) == ERROR){
		syslog(LOG_ERR, "Error eliminando semaforo 1 pool procesos");
	}
	if(Borrar_Semaforo(pool->extraProcNeededId) == ERROR){
		syslog(LOG_ERR, "Error eliminando semaforo 1 pool procesos");
	}
	return;	
}