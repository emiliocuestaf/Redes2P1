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


typedef struct _thrpool{
	thread* threadlist;
	//Habria que definir la estructura thread
} thrpool;



//Esto se puede plantear como un TAD
//addthread
//killthread (no creo que sea necesaria)
//int activethreadsnum
//...