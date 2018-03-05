#ifndef HANDLERS_H
	#define HANDLERS_H

#include "http_processing.h"

//Funciones publicas
int handler(char* bufferIn, char* bufferOut);

//Funciones privadas
int get_handler(char*bufferIn, char* bufferOut);
int post_handler(char*bufferIn, char* bufferOut);
int option_handler(char*bufferIn, char* bufferOut);
int php_script_handler(char*bufferIn, char* bufferOut);
int py_script_handler(char*bufferIn, char* bufferOut);

//Algunas mas de particion de archivos y esas cosas imagino

#endif

/********
* FUNCIÓN: int demonizar()
* ARGS_IN: 
* DESCRIPCIÓN: Procesa la petición de entrada, y blah, blah...
* ARGS_OUT: char * - devuelve un puntero señalado el verbo de la petición recibida
********/