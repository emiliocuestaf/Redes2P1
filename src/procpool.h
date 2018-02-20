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
#include "semaforos.h"


typedef struct _thread{
	int pid;
	int dad;
} process;

typedef struct _thrpool{
	int maxproc;
	int minproc;
	int activeprocs;
	process* proclist;
	//Habria que definir la estructura thread
} processpool;

processpool* pool_ini(int minproc, int maxproc);



//Esto se puede plantear como un TAD
//addthread
//killthread (no creo que sea necesaria)
//int activethreadsnum
//...


//Como garantizar que siempre hay minnum de procesos?

el servidor recibe peticion semup
un hijo se la pide semdown
el hijo acaba semup
ahora 2 opciones:

1) El hijo finaliza
   Proceso papa:
   while numproc < min proc
   	Se hace un nuevo fork (de esa manera siempre se garantiza el minimo)

   Mucha demanda:
   if sem > minproc
   	fork()


estados:

10 peticiones sin atender--semaforo = 10
si esta en 0, no entra
el semaforo me indica cuantas peticiones estan sin atender, como puedo sumar ahi? el servidor detecta cuando tiene una nueva peticion antes del accept?

//Tareas
	//Server:
		garantizar minimo numero de procesos paralelos 
		generar mas si fuera necesario
		subir el semaforo conforme llegan las peticiones
	//Hijos
		procesar la peticion y acabar

