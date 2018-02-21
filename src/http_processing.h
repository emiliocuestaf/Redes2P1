#ifndef HTTP_PROCESSING_H
	#define HTTP_PROCESSING_H


#include <stdio.h>
#include <stdlib.h>


/*Funcion que parsea una peticion HTTP*/

int parse_petition(char* inBuffer, char* outBuffer);
/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer);

#endif