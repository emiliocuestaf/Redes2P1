/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Modulo que se encarga de procesar una peticion HTTP y responderla
********************************************************/

#ifndef HTTP_PROCESSING_H
	#define HTTP_PROCESSING_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include "picohttpparser.h"
#include "socket_management.h"

#define DIREC_SIZE 200
#define COMMAND_SIZE 300

#define NO_SCRIPT 0
#define PYTHON_SCRIPT 1
#define PHP_SCRIPT 2 

#define ERROR -1
#define OK 0

/*Estructura que creamos para guardar conjuntos de metodos permitidos*/

typedef struct allowedMethods{
  int nummethods;
  char methods[3][10];
  char txtChain[30];
} allowedMethods;

/********
* FUNCIÓN:  char* get_date()
* DESCRIPCIÓN: Funcion que genera la fecha actual
* ARGS_OUT: char* - la fecha actual, NULL en caso de error
********/

char* get_date();

/********
* FUNCIÓN:  get_mod_time(struct stat* fStat)
* ARGS_IN: struct stat* fStat - estructura que contiene informacion del fichero
* DESCRIPCIÓN: Funcion que devuelve la fecha de la ultima modificacion de un fichero
* ARGS_OUT: char* - la fecha modificada, NULL en caso de error
********/

char* get_mod_time(struct stat* fStat);

/********
* FUNCIÓN:  char *filename_ext(char *fname)
* ARGS_IN: char* fname - nombre del fichero
* DESCRIPCIÓN: Funcion que devuelve el Content-Type de un fichero, a excepcion de los 
*              ficheros .py o .php cuyo retorno es su misma extension
* ARGS_OUT: char* - el Content-Type del fichero, "" si no está soportado
********/

char *filename_ext(char *fname);

/********
* FUNCIÓN:  int allowed_methods(allowedMethods* met, char* cleanpath)
* ARGS_IN:  àllowedMethods* met - estructura donde guardaremos los metodos que podemos aplicar en la ruta
            char* cleanpath - ruta que analizamos
* DESCRIPCIÓN: Devuelve los metodos validos en cada carpeta
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int allowed_methods(allowedMethods* met, char* cleanpath);

/********
* FUNCIÓN:  int parse_petition(int csock, char* inBuffer, char* signature, char* root, long int buf_size, long int timeout)
* ARGS_IN:  int csock - descriptor del socket del servidor
            char* inBuffer - peticion a parsear
            char* signature - nombre del servidor
            char* root - directorio raiz del servidor
            long int buf_size - longitud maxima del buffer de entrada/salida
            long int timeout - timeout maximo del servidor
* DESCRIPCIÓN: Parsea una peticion HTTP y crea la respuesta con funciones auxiliares
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int parse_petition(int clientsock, char* inBuffer, char* signature, char* root, long int buf_size, long int timeout);

/********
* FUNCIÓN:  int get_response(char* server_signature, int csock, char* direc, char* cleanpath, char* args, int max_buffer, int minor_version, long int timeout)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            char* direc - direccion del archivo completa
            char* cleanpath - archivo del fichero (sin aniadir el directorio raiz)
            char* args - argumentos del script
            long int max_buffer - longitud maxima del buffer de entrada/salida
            int minor_version - version de HTTP (1.0 o 1.1)
            long int timeout - timeout maximo del servidor
* DESCRIPCIÓN: responde a una peticion GET
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int get_response(char* server_signature, int clientsock, char* direc, char* cleanpath, char* args, int buffer_size, int minor_version, long int timeout);

/********
* FUNCIÓN:  int post_response(char* server_signature, int csock, char* direc, char* cleanpath, char* body, char* args_url, int max_buffer, int minor_version, long int timeout)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            char* direc - direccion del archivo completa
            char* cleanpath - archivo del fichero (sin aniadir el directorio raiz)
            char* body - cuerpo de la peticion (donde hay argumentos de stdin para el script)
            char* args_url - argumentos del script pasados por el URL
            long int max_buffer - longitud maxima del buffer de entrada/salida
            int minor_version - version de HTTP (1.0 o 1.1)
            long int timeout - timeout maximo del servidor
* DESCRIPCIÓN: Responde una peticion POST, ejecuta scripts unicamente
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int post_response(char* server_signature, int clientsock, char* direc, char* cleanpath, char* body, char* args_url, int buffer_size, int minor_version, long int timeout);

/********
* FUNCIÓN:  int options_response(char* server_signature, int csock, int buffer_size, int minor_version, allowedMethods* am)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            long int buffer_size - longitud maxima del buffer de entrada/salida
            int minor_version - version de HTTP (1.0 o 1.1)
            allowedMethods* am - metodos permitidos en este directorio
* DESCRIPCIÓN: Responde una peticion OPTIONS
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int options_response(char* server_signature, int clientsock, int buffer_size, int minor_version, allowedMethods* am);

/********
* FUNCIÓN:  int error_response(char* server_signature, int csock, char* cleanpath, int errnum, int minor_version, int buf_size)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            char* cleanpath - ruta del archivo sin el directorio raiz
            int errnum - numero de error
            int minor_version - version de HTTP (1.0 o 1.1)
            long int buf_size - longitud maxima del buffer de entrada/salida
* DESCRIPCIÓN: Genera una respuesta de error
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int error_response(char* server_signature, int clientsock, char* cleanpath, int errnum, int minor_version, int buf_size);

#endif
