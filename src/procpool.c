/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Funciones principales de un pool de threads
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
#include "confuse.h"
#include "socket_management.h"
#include "procpool.h"

typedef struct _thread{
	int pid;
	int dad;
} proccess;

typedef struct _thrpool{
	int maxproc;
	int minproc;
	int activeprocs;
	sema* proclist;
	//Habria que definir la estructura thread
} proccesspool;

proccess* pool_ini(int minproc, int maxproc){
	proccess pool;

	pool.minproc = minproc;
	pool.maxproc = maxproc;
	pool.activeprocs = minproc;
	pool.proclist = (proc *)malloc(pool.minproc*sizeof(proccess));


}


