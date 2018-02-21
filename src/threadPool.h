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

typedef struct _threadPool threadPool;

threadPool* pool_ini(int numProc, int listeningSocketDescr, int(*handler_pointer)(char*, char*));
void pool_free(threadPool* pool);

#endif