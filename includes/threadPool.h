/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Funciones principales de un pool estatico de threads
********************************************************/

#ifndef PROCPOOL_H
   #define PROCPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <pthread.h>
#include "socket_management.h"

typedef struct _threadPool threadPool;

threadPool* pool_ini(int numProc, int listeningSocketDescr, int buffSize, int(*handler_pointer)(int, char*));
void pool_free(threadPool* pool);

#endif