#ifndef HTTP_PROCESSING_H
	#define HTTP_PROCESSING_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ERROR 1
#define OK 0


typedef struct allowedMethods{
  int nummethods;
  char methods[3][10];
  char txtChain[30];
} allowedMethods;


char* get_date();

int error_response(char* server_signature, int clientsock, char* cleanpath, char* outBuffer, int errnum, int minor_version);

char* get_mod_time(struct stat* fStat);

char *filename_ext(char *fname);

int get_response(char* server_signature, int clientsock, char* direc, char* cleanpath, char* args, int buffer_size,  char* outBuffer, int minor_version);
int options_response(char* server_signature, int clientsock, char*outBuffer, int minor_version, allowedMethods* am);
int post_response(char* server_signature, int clientsock, char* direc, char* cleanpath, char* body, char* args_url, int buffer_size,  char* outBuffer, int minor_version);
/*Funcion que parsea una peticion HTTP*/

int parse_petition(int clientsock, char* inBuffer, char* outBuffer, char* signature, char* root, long int buf_size);
/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer);

#endif
