#ifndef HTTP_PROCESSING_H
	#define HTTP_PROCESSING_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ERROR 1
#define OK 0

char* get_date();

int error_response(char* outBuffer, int errnum, int minor_version);

char* get_mod_time(struct stat* fStat);

char *filename_ext(char *fname);

int get_response(char* outBuffer, int minor_version);

/*Funcion que parsea una peticion HTTP*/

int parse_petition(int clientsock, char* inBuffer, char* outBuffer, char* signature, char* root, long int buf_size);
/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer);

#endif
