/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Funciones principales de un pool de threads
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
#include "socket_management.h"
#include "semaforos.h"
#include "types.h"

typedef struct _processPool processPool;

processPool* pool_ini(int minProc, int maxProc, int n, int listeningSocketDescr, void(*handler_pointer)(char*, char*));

void pool_free(processPool* pool);

#endif