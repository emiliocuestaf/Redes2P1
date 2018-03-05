/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Cabeceras de funciones principales de un pool estatico de threads
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

/********
* ESTRUCTURA: threadPool
* DESCRIPCIÓN: Estructura que almacena informacion de un pool estatico de hilos mientras este se esta ejecutando
********/
typedef struct _threadPool threadPool;


/********
* FUNCIÓN:  threadPool* pool_ini(int numThr, int listeningSocketDescr, int buffSize, int(*handler_pointer)(int, char*));
* ARGS_IN:  int numThr - numero de hilos que va a mantener constantemente abiertos. 
            int listeningSocketDescr - socket que esta escuchando por el puerto, a trave de el se aceptan las peticiones.
            int buffSize - tamaño del buffer de las peticiones tanto de entrada como de respuesta.
            int(*handler_pointer)(int, char*) - puntero a la funcion que determinara el manejo de las peticiones recibidas.
* DESCRIPCIÓN: Funcion que sirve para inicializar un thread Pool estatico.
* ARGS_OUT: Un puntero al pool de threads creado o NULL en caso de error.
********/
threadPool* pool_ini(int numThr, int listeningSocketDescr, int buffSize, int(*handler_pointer)(int, char*));

/********
* FUNCIÓN:  void pool_free(threadPool* pool);  
* ARGS_IN:  threadPool* pool - pool de hilos que se va a liberar, cancelando todos los hilos.             
* DESCRIPCIÓN: Funcion que sirve para liberar un thread Pool estatico.
* ARGS_OUT: void
********/
void pool_free(threadPool* pool);

#endif